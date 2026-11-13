param(
    [Parameter(Mandatory=$true)][string]$MySqlExecutable,
    [Parameter(Mandatory=$true)][ValidateSet(33555,33556)][int]$Port,
    [switch]$KeepDatabases
)
$ErrorActionPreference = 'Stop'
$taskRoot = (Resolve-Path (Join-Path $PSScriptRoot '../..')).Path
$taskCli = (Resolve-Path -LiteralPath $MySqlExecutable).Path
$taskSuffix = "${PID}_$([DateTimeOffset]::UtcNow.ToUnixTimeSeconds())"
$taskAuth = "mop_test_battlepay_auth_$taskSuffix"
$taskCharacters = "mop_test_battlepay_chars_$taskSuffix"
$script:taskChecks = 0

function Start-TestSql([string]$Sql, [string]$Database = '') {
    $start = [Diagnostics.ProcessStartInfo]::new()
    $start.FileName = $taskCli
    $start.UseShellExecute = $false
    $start.CreateNoWindow = $true
    $start.RedirectStandardInput = $true
    $start.RedirectStandardOutput = $true
    $start.RedirectStandardError = $true
    $start.StandardInputEncoding = [Text.UTF8Encoding]::new($false)
    foreach ($argument in @('--no-defaults','--protocol=TCP','--host=127.0.0.1',"--port=$Port",'--user=root','--batch','--skip-column-names','--raw','--default-character-set=utf8mb4')) {
        $start.ArgumentList.Add($argument)
    }
    if ($Database) { $start.ArgumentList.Add("--database=$Database") }
    $process = [Diagnostics.Process]::new()
    $process.StartInfo = $start
    [void]$process.Start()
    $process.StandardInput.Write($Sql)
    $process.StandardInput.Close()
    return $process
}

function Complete-TestSql($Process, [switch]$ExpectFailure) {
    $output = $Process.StandardOutput.ReadToEnd()
    $errors = $Process.StandardError.ReadToEnd()
    $Process.WaitForExit()
    $failed = $Process.ExitCode -ne 0
    if ($failed -ne [bool]$ExpectFailure) { throw "Unexpected SQL result (exit $($Process.ExitCode)): $errors" }
    $Process.Dispose()
    return $output.Trim()
}

function Invoke-TestSql([string]$Sql, [string]$Database = '', [switch]$ExpectFailure) {
    Complete-TestSql (Start-TestSql $Sql $Database) -ExpectFailure:$ExpectFailure
}

function Assert-Sql([string]$Name,[string]$Sql,[string]$Expected,[string]$Database) {
    $actual = Invoke-TestSql $Sql $Database
    if ($actual -ne $Expected) { throw "$Name expected '$Expected', found '$actual'" }
    $script:taskChecks++
    Write-Output "PASS $Name"
}

function Get-ProductionQuery([string]$Name, [object[]]$Values, [string]$Header = 'BattlePayQueries.h') {
    $header = Get-Content -Raw -LiteralPath (Join-Path $taskRoot "src/server/game/BattlePay/$Header")
    $match = [regex]::Match($header, ('(?s)constexpr char '+$Name+'\[\] = R"SQL\((.*?)\)SQL";'))
    if (!$match.Success) { throw "Missing production query $Name" }
    $query = $match.Groups[1].Value
    $pattern = [regex]'%llu|%u'
    for ($index=0; $index -lt $Values.Count; $index++) {
        $query = $pattern.Replace($query, ([string]$Values[$index]), 1)
    }
    if ($pattern.IsMatch($query)) { throw "Missing query parameter for $Name" }
    return $query+';'
}

try {
    Write-Output ('Database server: '+(Invoke-TestSql 'SELECT VERSION();'))
    [void](Invoke-TestSql "CREATE DATABASE $taskAuth CHARACTER SET utf8; CREATE DATABASE $taskCharacters CHARACTER SET utf8;")
    [void](Invoke-TestSql @'
CREATE TABLE account (id INT UNSIGNED PRIMARY KEY,dp INT NOT NULL DEFAULT 0) ENGINE=InnoDB;
CREATE TABLE account_boost (id INT NOT NULL,realmid INT UNSIGNED NOT NULL,counter INT UNSIGNED NOT NULL) ENGINE=InnoDB;
CREATE TABLE battlepay_log (id INT UNSIGNED AUTO_INCREMENT PRIMARY KEY,accountId INT UNSIGNED,characterGuid INT,realm INT UNSIGNED,item INT,price INT UNSIGNED) ENGINE=InnoDB;
CREATE TABLE wow_token (id INT UNSIGNED AUTO_INCREMENT PRIMARY KEY,accountId INT UNSIGNED,characterGuid INT,realm INT UNSIGNED,coins INT UNSIGNED) ENGINE=InnoDB;
INSERT INTO account VALUES(1,100000),(2,0),(3,-1),(4,2147483647);
'@ $taskAuth)
    [void](Invoke-TestSql @'
CREATE TABLE characters (guid INT UNSIGNED PRIMARY KEY,account INT UNSIGNED NOT NULL) ENGINE=InnoDB;
CREATE TABLE mail (id INT UNSIGNED PRIMARY KEY,receiver INT UNSIGNED NOT NULL) ENGINE=InnoDB;
CREATE TABLE mail_items (mail_id INT UNSIGNED,item_guid INT UNSIGNED,receiver INT UNSIGNED,PRIMARY KEY(mail_id,item_guid)) ENGINE=InnoDB;
CREATE TABLE item_instance (guid INT UNSIGNED PRIMARY KEY,owner_guid INT UNSIGNED,count INT UNSIGNED) ENGINE=InnoDB;
INSERT INTO characters VALUES(10,1);
'@ $taskCharacters)
    $authMigration = Get-Content -Raw -LiteralPath (Join-Path $taskRoot 'sql/updates/auth/2026_10_28_00_battlepay_accounting.sql')
    $charMigration = Get-Content -Raw -LiteralPath (Join-Path $taskRoot 'sql/updates/characters/2026_10_28_00_battlepay_accounting.sql')
    [void](Invoke-TestSql $authMigration $taskAuth)
    [void](Invoke-TestSql $authMigration $taskAuth)
    [void](Invoke-TestSql $charMigration $taskCharacters)
    [void](Invoke-TestSql $charMigration $taskCharacters)
    $boostAuthMigration = Get-Content -Raw -LiteralPath (Join-Path $taskRoot 'sql/updates/auth/2026_10_28_01_battlepay_boost_recovery.sql')
    $boostCharacterMigration = Get-Content -Raw -LiteralPath (Join-Path $taskRoot 'sql/updates/characters/2026_10_28_01_battlepay_boost_recovery.sql')
    [void](Invoke-TestSql $boostAuthMigration $taskAuth)
    [void](Invoke-TestSql $boostAuthMigration $taskAuth)
    [void](Invoke-TestSql $boostCharacterMigration $taskCharacters)
    [void](Invoke-TestSql $boostCharacterMigration $taskCharacters)
    Assert-Sql 'repeatable migrations preserve existing DP' 'SELECT dp FROM account WHERE id=1;' '100000' $taskAuth
    Assert-Sql 'auth schema marker' 'SELECT version FROM battlepay_schema WHERE id=1;' '1' $taskAuth
    Assert-Sql 'character schema marker' 'SELECT version FROM battlepay_schema WHERE id=1;' '1' $taskCharacters
    [void](Invoke-TestSql @'
INSERT INTO battlepay_purchase(id,nonce,account_id,realm_id,character_guid,product_id,product_type,price,title,description) VALUES
(1,'00000000000000000000000000000001',1,1,10,13,0,70000,'fixture','fixture'),
(2,'00000000000000000000000000000002',1,1,10,13,0,70000,'fixture','fixture'),
(3,'00000000000000000000000000000003',2,1,10,13,0,1,'fixture','fixture'),
(4,'00000000000000000000000000000004',3,1,10,13,0,1,'fixture','fixture'),
(5,'00000000000000000000000000000005',1,1,10,13,0,10000,'fixture','fixture'),
(6,'00000000000000000000000000000006',2,1,10,13,0,0,'fixture','fixture'),
(7,'00000000000000000000000000000007',1,1,10,83,2,10000,'fixture','fixture');
UPDATE battlepay_purchase SET created_at=CURRENT_TIMESTAMP-INTERVAL 6 MINUTE WHERE id=5;
'@ $taskAuth)
    [void](Invoke-TestSql "INSERT INTO account VALUES(6,100); INSERT INTO battlepay_purchase(id,nonce,account_id,realm_id,character_guid,product_id,product_type,price,title,description) VALUES(10,'00000000000000000000000000000010',6,1,10,13,0,25,'whole DP','whole DP');" $taskAuth)
    $wholeDpCharge = Get-ProductionQuery 'Charge' @(10,6,1)
    [void](Invoke-TestSql $wholeDpCharge $taskAuth)
    Assert-Sql '25 DP product debits exactly 25 raw account DP' 'SELECT dp FROM account WHERE id=6;' '75' $taskAuth
    [void](Invoke-TestSql $wholeDpCharge $taskAuth)
    Assert-Sql 'repeated whole DP confirmation cannot double debit' 'SELECT dp FROM account WHERE id=6;' '75' $taskAuth
    $charge1 = Get-ProductionQuery 'Charge' @(1,1,1)
    $charge2 = Get-ProductionQuery 'Charge' @(2,1,1)
    $first = Start-TestSql ("SELECT SLEEP(0.2);"+$charge1) $taskAuth
    $second = Start-TestSql ("SELECT SLEEP(0.2);"+$charge2) $taskAuth
    [void](Complete-TestSql $first)
    [void](Complete-TestSql $second)
    Assert-Sql 'concurrent purchases cannot overspend' 'SELECT dp FROM account WHERE id=1;' '30000' $taskAuth
    Assert-Sql 'only one concurrent purchase charged' 'SELECT COUNT(*) FROM battlepay_purchase WHERE id IN(1,2) AND state=1;' '1' $taskAuth
    [void](Invoke-TestSql ($charge1+$charge2) $taskAuth)
    Assert-Sql 'repeated charge is idempotent' 'SELECT dp FROM account WHERE id=1;' '30000' $taskAuth
    [void](Invoke-TestSql (Get-ProductionQuery 'Charge' @(3,2,1)) $taskAuth)
    Assert-Sql 'insufficient funds rejected' 'SELECT state FROM battlepay_purchase WHERE id=3;' '0' $taskAuth
    [void](Invoke-TestSql (Get-ProductionQuery 'Charge' @(4,3,1)) $taskAuth)
    Assert-Sql 'negative DP rejected without unsigned wrap' 'SELECT dp FROM account WHERE id=3;' '-1' $taskAuth
    Assert-Sql 'negative wallet did not authorize purchase' 'SELECT state FROM battlepay_purchase WHERE id=4;' '0' $taskAuth
    [void](Invoke-TestSql (Get-ProductionQuery 'Charge' @(5,1,1)) $taskAuth)
    Assert-Sql 'expired quotes cannot charge' 'SELECT state FROM battlepay_purchase WHERE id=5;' '0' $taskAuth
    [void](Invoke-TestSql (Get-ProductionQuery 'Charge' @(6,2,1)) $taskAuth)
    Assert-Sql 'free product records paid state with zero balance' 'SELECT state FROM battlepay_purchase WHERE id=6;' '1' $taskAuth
    [void](Invoke-TestSql ('UPDATE account SET dp=100000 WHERE id=1;'+$charge1+$charge2) $taskAuth)
    Assert-Sql 'external wallet changes affect next charge' 'SELECT dp FROM account WHERE id=1;' '30000' $taskAuth
    Assert-Sql 'external credit permits other purchase once' 'SELECT COUNT(*) FROM battlepay_purchase WHERE id IN(1,2) AND state=1;' '2' $taskAuth
    [void](Invoke-TestSql (Get-ProductionQuery 'Charge' @(7,2,1)) $taskAuth)
    Assert-Sql 'foreign account cannot charge purchase' 'SELECT state FROM battlepay_purchase WHERE id=7;' '0' $taskAuth

    $delivery = @'
START TRANSACTION;
INSERT INTO battlepay_delivery(purchase_id,account_id,character_guid,attempt,state) SELECT 1,1,guid,'00000000000000000000000000000001',1 FROM characters WHERE guid=10 AND account=1;
INSERT INTO item_instance VALUES(701,10,1);
INSERT INTO mail VALUES(501,10);
INSERT INTO mail_items VALUES(501,701,10);
COMMIT;
'@
    [void](Invoke-TestSql $delivery $taskCharacters)
    [void](Invoke-TestSql $delivery $taskCharacters -ExpectFailure)
    Assert-Sql 'duplicate delivery leaves one receipt' 'SELECT COUNT(*) FROM battlepay_delivery WHERE purchase_id=1;' '1' $taskCharacters
    Assert-Sql 'duplicate delivery leaves one item' 'SELECT COUNT(*) FROM item_instance;' '1' $taskCharacters
    [void](Invoke-TestSql @'
START TRANSACTION;
INSERT INTO battlepay_delivery VALUES(2,1,10,'00000000000000000000000000000002',1,CURRENT_TIMESTAMP);
INSERT INTO mail VALUES(501,10);
COMMIT;
'@ $taskCharacters -ExpectFailure)
    Assert-Sql 'delivery SQL failure rolls back its receipt' 'SELECT COUNT(*) FROM battlepay_delivery WHERE purchase_id=2;' '0' $taskCharacters

    $finalize = @'
START TRANSACTION;
INSERT INTO battlepay_service_delivery(purchase_id) VALUES(1);
INSERT INTO battlepay_log(accountId,characterGuid,realm,item,price) SELECT account_id,character_guid,realm_id,13,price FROM battlepay_purchase WHERE id=1 AND state=1;
UPDATE battlepay_purchase SET state=2 WHERE id=1 AND state=1;
COMMIT;
'@
    Assert-Sql 'interrupted purchase retains paid state until recovery' 'SELECT state FROM battlepay_purchase WHERE id=1;' '1' $taskAuth
    [void](Invoke-TestSql $finalize $taskAuth)
    [void](Invoke-TestSql $finalize $taskAuth -ExpectFailure)
    Assert-Sql 'recovery finalizes once' 'SELECT state FROM battlepay_purchase WHERE id=1;' '2' $taskAuth
    Assert-Sql 'recovery logs once' 'SELECT COUNT(*) FROM battlepay_log WHERE item=13;' '1' $taskAuth

    [void](Invoke-TestSql "INSERT INTO battlepay_delivery(purchase_id,account_id,character_guid,attempt,state) VALUES(2,1,10,'00000000000000000000000000000002',0);" $taskCharacters)
    $refund = Get-ProductionQuery 'Refund' @(2,1)
    [void](Invoke-TestSql ($refund+$refund) $taskAuth)
    Assert-Sql 'cancellation refund restores DP once' 'SELECT dp FROM account WHERE id=1;' '100000' $taskAuth
    Assert-Sql 'refunded purchase has durable terminal state' 'SELECT state FROM battlepay_purchase WHERE id=2;' '4' $taskAuth
    [void](Invoke-TestSql "INSERT INTO battlepay_delivery(purchase_id,account_id,character_guid,attempt,state) VALUES(2,1,10,'00000000000000000000000000000003',1);" $taskCharacters -ExpectFailure)
    Assert-Sql 'cancellation fence blocks later delivery' 'SELECT state FROM battlepay_delivery WHERE purchase_id=2;' '0' $taskCharacters

    [void](Invoke-TestSql (Get-ProductionQuery 'Charge' @(7,1,1)) $taskAuth)
    $boost = @'
START TRANSACTION;
INSERT INTO battlepay_service_delivery(purchase_id) VALUES(7);
INSERT INTO account_boost(id,realmid,counter) VALUES(1,1,1) ON DUPLICATE KEY UPDATE counter=counter+1;
INSERT INTO battlepay_log(accountId,characterGuid,realm,item,price) VALUES(1,10,1,0,10000);
UPDATE battlepay_purchase SET state=2 WHERE id=7 AND state=1;
COMMIT;
'@
    [void](Invoke-TestSql $boost $taskAuth)
    [void](Invoke-TestSql $boost $taskAuth -ExpectFailure)
    Assert-Sql 'boost grant survives replay without duplicate entitlement' 'SELECT counter FROM account_boost WHERE id=1 AND realmid=1;' '1' $taskAuth
    [void](Invoke-TestSql "INSERT INTO battlepay_token_credit(realm_id,item_guid,account_id,character_guid,amount) VALUES(1,901,2,10,10000);" $taskAuth)
    $token = Get-ProductionQuery 'TokenCredit' @(1,901,2,10000)
    Assert-Sql 'token credit changes wallet and receipt rows together' ($token+'SELECT ROW_COUNT();') '2' $taskAuth
    [void](Invoke-TestSql $token $taskAuth)
    Assert-Sql 'physical token receipt prevents repeat credit' 'SELECT dp FROM account WHERE id=2;' '10000' $taskAuth
    Assert-Sql 'token credit receipt records application' 'SELECT applied FROM battlepay_token_credit WHERE item_guid=901;' '1' $taskAuth
    [void](Invoke-TestSql "INSERT INTO battlepay_token_credit(realm_id,item_guid,account_id,character_guid,amount) VALUES(1,902,4,10,10000);" $taskAuth)
    [void](Invoke-TestSql (Get-ProductionQuery 'TokenCredit' @(1,902,4,10000)) $taskAuth)
    Assert-Sql 'token credit cannot overflow signed account DP' 'SELECT dp FROM account WHERE id=4;' '2147483647' $taskAuth
    Assert-Sql 'overflowed credit does not consume token receipt' 'SELECT applied FROM battlepay_token_credit WHERE item_guid=902;' '0' $taskAuth
    [void](Invoke-TestSql "INSERT INTO battlepay_purchase(id,nonce,account_id,realm_id,character_guid,product_id,product_type,price,title,description,state) VALUES(8,'00000000000000000000000000000008',4,1,10,13,0,10000,'fixture','fixture',1);" $taskAuth)
    [void](Invoke-TestSql (Get-ProductionQuery 'Refund' @(8,4)) $taskAuth)
    Assert-Sql 'refund overflow remains pending without lost amount' 'SELECT state FROM battlepay_purchase WHERE id=8;' '1' $taskAuth
    [void](Invoke-TestSql "ALTER TABLE account MODIFY dp INT NULL; INSERT INTO account VALUES(5,NULL); INSERT INTO battlepay_purchase(id,nonce,account_id,realm_id,character_guid,product_id,product_type,price,title,description) VALUES(9,'00000000000000000000000000000009',5,1,10,13,0,0,'fixture','fixture');" $taskAuth)
    [void](Invoke-TestSql (Get-ProductionQuery 'Charge' @(9,5,1)) $taskAuth)
    Assert-Sql 'NULL wallet fails closed even for free purchase' 'SELECT state FROM battlepay_purchase WHERE id=9;' '0' $taskAuth

    [void](Invoke-TestSql @'
INSERT INTO battlepay_boost_use(id,account_id,realm_id,entitlement_realm,character_guid,specialization,alliance) VALUES
(1,1,1,1,10,71,1),(2,1,1,1,11,71,1),(3,1,1,1,12,71,1);
'@ $taskAuth)
    $reserve1 = Get-ProductionQuery 'Reserve' @(1,1,1) 'BattlePayBoostQueries.h'
    [void](Invoke-TestSql ($reserve1+$reserve1) $taskAuth)
    Assert-Sql 'boost reservation consumes exactly one entitlement' 'SELECT counter FROM account_boost WHERE id=1 AND realmid=1;' '0' $taskAuth
    Assert-Sql 'boost reservation has durable reserved state' 'SELECT state FROM battlepay_boost_use WHERE id=1;' '1' $taskAuth
    [void](Invoke-TestSql (Get-ProductionQuery 'Reserve' @(2,1,1) 'BattlePayBoostQueries.h') $taskAuth)
    Assert-Sql 'zero boost counter cannot underflow' 'SELECT counter FROM account_boost WHERE id=1 AND realmid=1;' '0' $taskAuth
    Assert-Sql 'unfunded boost cannot reserve' 'SELECT state FROM battlepay_boost_use WHERE id=2;' '0' $taskAuth
    [void](Invoke-TestSql "INSERT INTO battlepay_boost_delivery(use_id,account_id,character_guid,state) VALUES(1,1,10,0);" $taskCharacters)
    $restore1 = Get-ProductionQuery 'Restore' @(1,1,1) 'BattlePayBoostQueries.h'
    [void](Invoke-TestSql ($restore1+$restore1) $taskAuth)
    Assert-Sql 'cancelled boost restores its entitlement once' 'SELECT counter FROM account_boost WHERE id=1 AND realmid=1;' '1' $taskAuth
    Assert-Sql 'restored boost has terminal state' 'SELECT state FROM battlepay_boost_use WHERE id=1;' '3' $taskAuth
    [void](Invoke-TestSql "INSERT INTO battlepay_boost_delivery(use_id,account_id,character_guid,state) VALUES(1,1,10,1);" $taskCharacters -ExpectFailure)
    Assert-Sql 'boost cancellation fence blocks later application' 'SELECT state FROM battlepay_boost_delivery WHERE use_id=1;' '0' $taskCharacters
    [void](Invoke-TestSql (Get-ProductionQuery 'Reserve' @(2,1,1) 'BattlePayBoostQueries.h') $taskAuth)
    [void](Invoke-TestSql "ALTER TABLE characters ADD level TINYINT UNSIGNED NOT NULL DEFAULT 1; INSERT INTO characters(guid,account) VALUES(11,1);" $taskCharacters)
    $boostApply = @'
START TRANSACTION;
INSERT INTO battlepay_boost_delivery(use_id,account_id,character_guid,state) VALUES(2,1,11,1);
UPDATE characters SET level=90 WHERE guid=11 AND account=1;
COMMIT;
'@
    [void](Invoke-TestSql $boostApply $taskCharacters)
    [void](Invoke-TestSql $boostApply $taskCharacters -ExpectFailure)
    Assert-Sql 'boost application receipt and character changes commit together' 'SELECT level FROM characters WHERE guid=11;' '90' $taskCharacters
    Assert-Sql 'boost retry cannot add a second application receipt' 'SELECT COUNT(*) FROM battlepay_boost_delivery WHERE use_id=2;' '1' $taskCharacters
    [void](Invoke-TestSql 'UPDATE battlepay_boost_use SET state=2 WHERE id=2 AND state=1;' $taskAuth)
    [void](Invoke-TestSql (Get-ProductionQuery 'Reserve' @(2,1,1) 'BattlePayBoostQueries.h') $taskAuth)
    Assert-Sql 'applied boost cannot consume another entitlement' 'SELECT counter FROM account_boost WHERE id=1 AND realmid=1;' '0' $taskAuth
    [void](Invoke-TestSql (Get-ProductionQuery 'Reserve' @(3,2,1) 'BattlePayBoostQueries.h') $taskAuth)
    Assert-Sql 'foreign account cannot reserve another boost request' 'SELECT state FROM battlepay_boost_use WHERE id=3;' '0' $taskAuth
    Write-Output "$script:taskChecks BattlePay database checks passed on loopback port $Port"
    Write-Output "Fixture databases: $taskAuth ; $taskCharacters"
}
finally {
    if (!$KeepDatabases) {
        foreach ($database in @($taskAuth,$taskCharacters)) {
            if ($database -notmatch '^mop_test_battlepay_(auth|chars)_[0-9]+_[0-9]+$') { throw 'Unsafe fixture cleanup name' }
            [void](Invoke-TestSql "DROP DATABASE IF EXISTS $database;")
        }
    }
}
