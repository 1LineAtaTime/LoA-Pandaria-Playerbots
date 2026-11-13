"""Regenerate standalone social-chat SQL from the reviewed plain-text source files."""
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
rows=[]
for name in ('ambient_general_548','party_flavor_548'):
    for line in (ROOT/f'modules/mod_playerbots/data/chat/{name}.txt').read_text(encoding='utf-8-sig').splitlines():
        if line.strip():
            rows.append((name,line.strip()))
assert len(rows)==115 and len({text for _,text in rows})==115
assert all(text.isascii() and len(text)<180 and '%' not in text and '|' not in text for _,text in rows)
def literal(text):
    return 'CONVERT(0x'+text.encode().hex()+' USING utf8mb4)'
sql=['-- Standalone conversational lines, not opener/coda combinations. Playerbots schema.']
for name,text in rows:
    sql.append('INSERT INTO ai_playerbot_texts(name,text,say_type,reply_type) SELECT '+literal(name)+','+literal(text)+',0,0 WHERE NOT EXISTS(SELECT 1 FROM ai_playerbot_texts WHERE BINARY name='+literal(name)+' AND BINARY text='+literal(text)+');')
(ROOT/'modules/mod_playerbots/data/sql/playerbots/2026_11_12_02_standalone_social_chat.sql').write_text('\n'.join(sql)+'\n',encoding='ascii')
print('Validated/generated115 unique standalone lines, ASCII and under180 characters')
