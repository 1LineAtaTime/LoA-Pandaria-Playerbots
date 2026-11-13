-- The EmuCoach auth schema had an empty build_info table. Without this row,
-- authserver rejects MoP 5.4.8 clients before checking account credentials.
-- Keep any existing build metadata untouched.
INSERT IGNORE INTO build_info
    (build, majorVersion, minorVersion, bugfixVersion, hotfixVersion,
     winAuthSeed, win64AuthSeed, mac64AuthSeed, winChecksumSeed, macChecksumSeed)
VALUES (18414, 5, 4, 8, NULL, NULL, NULL, NULL, NULL, NULL);
