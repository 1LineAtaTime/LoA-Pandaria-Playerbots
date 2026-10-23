-- #892: master pools whose children are rare-NPC pools for different entries in different zones (alexkulya lineage;
-- AC acore_world gives each rare its own mother-less max_limit-1 pool). Detach the children and drop the emptied masters.
DELETE FROM pool_pool WHERE mother_pool IN (5620,5621,5622,5623,5624,5625,5626,5628,5629,5630,5631,5632,5634,5635,5637,5638,5640,5641,5642,5643,5645,5647,5648,5649);
DELETE FROM pool_template WHERE entry IN (5620,5621,5622,5623,5624,5625,5626,5628,5629,5630,5631,5632,5634,5635,5637,5638,5640,5641,5642,5643,5645,5647,5648,5649) AND description='Master pool';
