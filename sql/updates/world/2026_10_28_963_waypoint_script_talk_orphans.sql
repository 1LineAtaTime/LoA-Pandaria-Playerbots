-- Issue #963: DBErrors.log cleanup, waypoint_scripts SCRIPT_COMMAND_TALK + orphan scripts.
-- WHAT:
--  (a) 103 waypoint_scripts TALK rows had dataint=0 ("invalid talk text id (dataint = 0)", 103 log lines).
--      Their text lives in the reference DB db_script_string (entry 2000000xxx). This core's Unit::Say(textId) reads
--      broadcast_text, so the strings are ported into broadcast_text under the same 2000000xxx ids (outside
--      the client broadcast range, no collision) and dataint is set from the reference DB waypoint_scripts (same guid/id).
--      Only the 89 rows whose script id is linked from waypoint_data.action are repaired.
--  (b) 46 distinct waypoint script ids (rows with no waypoint_data.action link, "There is no waypoint which
--      links to the waypoint script", 46 lines) belong to the reference DB paths for creature guids 5848/5856/5858/9310
--      that the world DB does not carry (MovementType 0 / different path). They never run; removed.
--      14 of those are also among the 103 TALK rows; they are deleted rather than repaired.
-- EXPECTED AFTER: 0 + 0 log lines for these two types.
-- SOURCE: reference DB db_script_string, reference DB waypoint_scripts (rows only).
-- Undo: restore from backup; broadcast_text rows are IDs >= 2000000000 only.

DELETE FROM `waypoint_scripts` WHERE `id` NOT IN (SELECT `action` FROM (SELECT DISTINCT `action` FROM `waypoint_data` WHERE `action` > 0) a);

DELETE FROM `broadcast_text` WHERE `ID` BETWEEN 2000000000 AND 2000009999;
INSERT INTO `broadcast_text` (`ID`,`LanguageID`,`Text`,`Text1`,`EmoteID1`,`EmoteID2`,`EmoteID3`,`EmoteDelay1`,`EmoteDelay2`,`EmoteDelay3`,`SoundEntriesID`,`EmotesID`,`Flags`,`VerifiedBuild`) VALUES
(2000000133, 0, 'Stormwind City is under reliable protection!', 'Stormwind City is under reliable protection!', 0,0,0,0,0,0,0,0,0,0),
(2000000134, 0, 'The enemy won`t pass!', 'The enemy won`t pass!', 0,0,0,0,0,0,0,0,0,0),
(2000000139, 0, 'Meal, drinks!', 'Meal, drinks!', 0,0,0,0,0,0,0,0,0,0),
(2000000142, 0, 'Ironforge is under reliable protection!', 'Ironforge is under reliable protection!', 0,0,0,0,0,0,0,0,0,0),
(2000000143, 0, 'How can anyone live like this!?', 'How can anyone live like this!?', 0,0,0,0,0,0,0,0,0,0),
(2000000144, 0, 'This must be fixed!', 'This must be fixed!', 0,0,0,0,0,0,0,0,0,0),
(2000000145, 0, 'This will never do..', 'This will never do..', 0,0,0,0,0,0,0,0,0,0),
(2000000146, 0, 'More Work?', 'More Work?', 0,0,0,0,0,0,0,0,0,0),
(2000000147, 0, 'I AM A GOD OF DEMONS', 'I AM A GOD OF DEMONS', 0,0,0,0,0,0,0,0,0,0),
(2000000148, 0, 'WHERE MY FORCES OF DEMONS', 'WHERE MY FORCES OF DEMONS', 0,0,0,0,0,0,0,0,0,0),
(2000000149, 0, 'I WILL DESTROY ALL', 'I WILL DESTROY ALL', 0,0,0,0,0,0,0,0,0,0),
(2000000150, 0, 'US ALREADY ALL ANYMORE AND ANYMORE', 'US ALREADY ALL ANYMORE AND ANYMORE', 0,0,0,0,0,0,0,0,0,0),
(2000000151, 0, 'Welcom to piratet Harbor !!', 'Welcom to piratet Harbor !!', 0,0,0,0,0,0,0,0,0,0),
(2000000152, 0, 'Greatings my soldier !!', 'Greatings my soldier !!', 0,0,0,0,0,0,0,0,0,0),
(2000000153, 0, 'My blade - Orcs head down !!', 'My blade - Orcs head down !!', 0,0,0,0,0,0,0,0,0,0),
(2000000154, 0, 'Hm.. I filing you, come and fight !!', 'Hm.. I filing you, come and fight !!', 0,0,0,0,0,0,0,0,0,0),
(2000000155, 0, 'Uhh.. Nice girl !!', 'Uhh.. Nice girl !!', 0,0,0,0,0,0,0,0,0,0),
(2000000156, 0, 'Tanaris is our lend !! ZZzzZZ', 'Tanaris is our lend !! ZZzzZZ', 0,0,0,0,0,0,0,0,0,0),
(2000000157, 0, 'Prizes! Get your prizes right here! Get over here and turn in your Darkmoon Faire Prize Tickets for valuable prizes!', 'Prizes! Get your prizes right here! Get over here and turn in your Darkmoon Faire Prize Tickets for valuable prizes!', 0,0,0,0,0,0,0,0,0,0),
(2000000164, 0, 'Ahh, my precious Ameenah! How wonderful to see you again.', 'Ahh, my precious Ameenah! How wonderful to see you again.', 0,0,0,0,0,0,0,0,0,0),
(2000000165, 0, 'Yer wearin down, princess, I can sense it!', 'Yer wearin down, princess, I can sense it!', 0,0,0,0,0,0,0,0,0,0),
(2000000166, 0, 'Hmm, don\'t mind if I do!', 'Hmm, don\'t mind if I do!', 0,0,0,0,0,0,0,0,0,0),
(2000000183, 0, 'I\'ll never stop. Never...', 'I\'ll never stop. Never...', 0,0,0,0,0,0,0,0,0,0),
(2000000194, 0, 'Poor Dorius. If I ever get my hands on those Dark Irons, so help me...', 'Poor Dorius. If I ever get my hands on those Dark Irons, so help me...', 0,0,0,0,0,0,0,0,0,0),
(2000000196, 0, 'I know you are there, rougue, Leave my home or join the others at the bottom of the world tree.', 'I know you are there, rougue, Leave my home or join the others at the bottom of the world tree.', 0,0,0,0,0,0,0,0,0,0),
(2000000197, 0, 'Filth! Filth everywhere! The forests must be Cleansed', 'Filth! Filth everywhere! The forests must be Cleansed', 0,0,0,0,0,0,0,0,0,0),
(2000000201, 0, 'Duck!', 'Duck!', 0,0,0,0,0,0,0,0,0,0),
(2000000202, 0, 'Don\'t go close to the lake! It\'s haunted!', 'Don\'t go close to the lake! It\'s haunted!', 0,0,0,0,0,0,0,0,0,0),
(2000000203, 0, 'I will do my best to take care of the children, Grandmother.', 'I will do my best to take care of the children, Grandmother.', 0,0,0,0,0,0,0,0,0,0),
(2000000204, 0, 'Well hand it overand let\'s see what you got!', 'Well hand it overand let\'s see what you got!', 0,0,0,0,0,0,0,0,0,0),
(2000000205, 0, 'Great Gazlowe!', 'Great Gazlowe!', 0,0,0,0,0,0,0,0,0,0),
(2000000206, 0, 'I have another reading from the nether.', 'I have another reading from the nether.', 0,0,0,0,0,0,0,0,0,0),
(2000000207, 0, 'Right. We just busted our tails to haul all of this stuff up here, and you want us to work more?', 'Right. We just busted our tails to haul all of this stuff up here, and you want us to work more?', 0,0,0,0,0,0,0,0,0,0),
(2000000208, 0, 'The wind speaks of our enemies\' movements, Ashyen. The Naga will attack again.', 'The wind speaks of our enemies\' movements, Ashyen. The Naga will attack again.', 0,0,0,0,0,0,0,0,0,0),
(2000000209, 0, 'Very well. When that decision is made, I will be ready. Let us hope it won\'t be too late.', 'Very well. When that decision is made, I will be ready. Let us hope it won\'t be too late.', 0,0,0,0,0,0,0,0,0,0),
(2000000221, 0, 'Approach, buy.', 'Approach, buy.', 0,0,0,0,0,0,0,0,0,0),
(2000000223, 0, 'The back walls are completely blown out!', 'The back walls are completely blown out!', 0,0,0,0,0,0,0,0,0,0),
(2000000224, 0, 'They call that a weld!..', 'They call that a weld!..', 0,0,0,0,0,0,0,0,0,0),
(2000000225, 0, 'This must be fixed..', 'This must be fixed..', 0,0,0,0,0,0,0,0,0,0),
(2000000226, 0, 'I WILL TAKE ALL LIFES', 'I WILL TAKE ALL LIFES', 0,0,0,0,0,0,0,0,0,0),
(2000000227, 0, 'I FEEL THE FRIED MEAT', 'I FEEL THE FRIED MEAT', 0,0,0,0,0,0,0,0,0,0),
(2000000228, 0, 'YOU PRAY SERVANTS MY', 'YOU PRAY SERVANTS MY', 0,0,0,0,0,0,0,0,0,0),
(2000000229, 0, 'Tickets! Redeem your Darkmoon Faire Prize Tickets here! Wondrous and exotic prizes are waiting for you!', 'Tickets! Redeem your Darkmoon Faire Prize Tickets here! Wondrous and exotic prizes are waiting for you!', 0,0,0,0,0,0,0,0,0,0),
(2000000234, 0, 'This data will be quite usefull for Project Blue Book. Er, I mean this will be great for my new look. See if you can gather any more data.', 'This data will be quite usefull for Project Blue Book. Er, I mean this will be great for my new look. See if you can gather any more data.', 0,0,0,0,0,0,0,0,0,0),
(2000000235, 0, 'I WICKED OGR', 'I WICKED OGR', 0,0,0,0,0,0,0,0,0,0),
(2000000236, 0, 'I FEEL DEATH ALONGSIDE', 'I FEEL DEATH ALONGSIDE', 0,0,0,0,0,0,0,0,0,0),
(2000000238, 0, 'I WILL DESTROY YOU ALL', 'I WILL DESTROY YOU ALL', 0,0,0,0,0,0,0,0,0,0),
(2000000239, 0, 'NOBODY WILL GET THROUGH ME', 'NOBODY WILL GET THROUGH ME', 0,0,0,0,0,0,0,0,0,0);

-- set dataint per script row (values from the reference DB waypoint_scripts, same guid + id)
UPDATE `waypoint_scripts` SET `dataint` = 2000000142 WHERE `guid` = 1 AND `id` = 1 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000157 WHERE `guid` = 2 AND `id` = 24 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000134 WHERE `guid` = 50 AND `id` = 100 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000133 WHERE `guid` = 51 AND `id` = 101 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000134 WHERE `guid` = 98 AND `id` = 154 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000194 WHERE `guid` = 99 AND `id` = 156 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000196 WHERE `guid` = 140 AND `id` = 212 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000197 WHERE `guid` = 141 AND `id` = 213 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000139 WHERE `guid` = 142 AND `id` = 214 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000139 WHERE `guid` = 143 AND `id` = 215 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000139 WHERE `guid` = 144 AND `id` = 216 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000139 WHERE `guid` = 145 AND `id` = 217 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000139 WHERE `guid` = 146 AND `id` = 218 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000139 WHERE `guid` = 147 AND `id` = 219 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000139 WHERE `guid` = 148 AND `id` = 220 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000139 WHERE `guid` = 149 AND `id` = 221 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000139 WHERE `guid` = 150 AND `id` = 222 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000139 WHERE `guid` = 151 AND `id` = 223 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000139 WHERE `guid` = 152 AND `id` = 224 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000139 WHERE `guid` = 153 AND `id` = 225 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000139 WHERE `guid` = 154 AND `id` = 226 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000139 WHERE `guid` = 155 AND `id` = 227 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000139 WHERE `guid` = 156 AND `id` = 228 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000139 WHERE `guid` = 157 AND `id` = 229 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000139 WHERE `guid` = 158 AND `id` = 230 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000139 WHERE `guid` = 159 AND `id` = 231 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000134 WHERE `guid` = 160 AND `id` = 232 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000147 WHERE `guid` = 245 AND `id` = 332 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000148 WHERE `guid` = 246 AND `id` = 333 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000149 WHERE `guid` = 247 AND `id` = 336 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000150 WHERE `guid` = 248 AND `id` = 337 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000204 WHERE `guid` = 250 AND `id` = 351 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000205 WHERE `guid` = 251 AND `id` = 352 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000183 WHERE `guid` = 252 AND `id` = 354 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000207 WHERE `guid` = 253 AND `id` = 426 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000208 WHERE `guid` = 254 AND `id` = 427 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000209 WHERE `guid` = 255 AND `id` = 428 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000201 WHERE `guid` = 256 AND `id` = 443 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000206 WHERE `guid` = 257 AND `id` = 446 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000202 WHERE `guid` = 258 AND `id` = 460 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000146 WHERE `guid` = 259 AND `id` = 485 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000203 WHERE `guid` = 264 AND `id` = 491 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000145 WHERE `guid` = 265 AND `id` = 499 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000144 WHERE `guid` = 266 AND `id` = 500 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000143 WHERE `guid` = 267 AND `id` = 501 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000157 WHERE `guid` = 272 AND `id` = 537 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000164 WHERE `guid` = 273 AND `id` = 538 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000165 WHERE `guid` = 274 AND `id` = 539 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000166 WHERE `guid` = 275 AND `id` = 540 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000151 WHERE `guid` = 277 AND `id` = 567 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000152 WHERE `guid` = 278 AND `id` = 568 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000153 WHERE `guid` = 279 AND `id` = 569 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000154 WHERE `guid` = 280 AND `id` = 570 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000155 WHERE `guid` = 281 AND `id` = 571 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000156 WHERE `guid` = 282 AND `id` = 572 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000229 WHERE `guid` = 300 AND `id` = 24 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000221 WHERE `guid` = 343 AND `id` = 214 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000221 WHERE `guid` = 344 AND `id` = 215 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000221 WHERE `guid` = 345 AND `id` = 216 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000221 WHERE `guid` = 346 AND `id` = 217 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000221 WHERE `guid` = 347 AND `id` = 218 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000221 WHERE `guid` = 348 AND `id` = 219 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000221 WHERE `guid` = 349 AND `id` = 220 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000221 WHERE `guid` = 350 AND `id` = 221 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000221 WHERE `guid` = 351 AND `id` = 222 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000221 WHERE `guid` = 352 AND `id` = 223 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000221 WHERE `guid` = 353 AND `id` = 224 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000221 WHERE `guid` = 354 AND `id` = 225 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000221 WHERE `guid` = 355 AND `id` = 226 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000221 WHERE `guid` = 356 AND `id` = 227 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000221 WHERE `guid` = 357 AND `id` = 228 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000221 WHERE `guid` = 358 AND `id` = 229 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000221 WHERE `guid` = 359 AND `id` = 230 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000221 WHERE `guid` = 360 AND `id` = 231 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000226 WHERE `guid` = 401 AND `id` = 332 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000227 WHERE `guid` = 402 AND `id` = 334 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000228 WHERE `guid` = 403 AND `id` = 335 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000234 WHERE `guid` = 404 AND `id` = 352 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000225 WHERE `guid` = 405 AND `id` = 499 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000224 WHERE `guid` = 406 AND `id` = 500 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000223 WHERE `guid` = 407 AND `id` = 501 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000229 WHERE `guid` = 409 AND `id` = 537 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000235 WHERE `guid` = 412 AND `id` = 336 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000236 WHERE `guid` = 413 AND `id` = 337 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000238 WHERE `guid` = 415 AND `id` = 332 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000239 WHERE `guid` = 416 AND `id` = 333 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000239 WHERE `guid` = 417 AND `id` = 335 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000238 WHERE `guid` = 419 AND `id` = 334 AND `command` = 0 AND `dataint` = 0;
UPDATE `waypoint_scripts` SET `dataint` = 2000000236 WHERE `guid` = 420 AND `id` = 336 AND `command` = 0 AND `dataint` = 0;
