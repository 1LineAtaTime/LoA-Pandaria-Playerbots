# Auction house activity using random playerbots

The existing AuctionHouseBot still runs the market simulation. With
`AuctionHouseBot.UseRandomPlayerbots = 1`, it uses character GUIDs belonging to
the accounts named by `AiPlayerbot.RandomBotAccountPrefix` and
`AiPlayerbot.RandomBotAccountCount` for new listings and bids. Alliance houses
use Alliance characters, Horde houses use Horde characters, and neutral houses
can use either. The configured legacy `AuctionHouseBot.Account` and optional
`AuctionHouseBot.GUID` remain recognized as bot owners, so older listings are
not mistaken for player listings. New simulated listings use random bot names.

The random playerbot AI does not yet walk to an auctioneer or list items from
its inventory. The auction bot creates market stock and chooses an existing
random bot character as its visible seller or buyer. It buys and bids on
eligible player auctions using the existing price/chance rules; it skips
bot-owned listings. This distinction matters when tuning gold and item flow.

Before building the seller pool, the server excludes names with whole-word
markers `TEST`, `DEPRECATED`, `UNUSED`, `DEBUG`, `PLACEHOLDER`, `DNT`, `DNU`,
`QA`, `NYI`, `DO NOT USE`, and `NOT USED`, regardless of forced include IDs.
Names such as `Slatesteel` are unaffected. At startup, the server expires
existing unbid bot listings with these markers through the normal auction
expiry path; listings with a player bid are left alone.

The seller also applies `AuctionHouseBot.Items.ReqLevel.Max = 99` to every
item class and forced include. This honors the world database convention that
required level 100 or above marks an item as unsuitable for auction. It is
the required level limit; item level is a separate setting and remains
unlimited so legitimate high item level Pandaria gear can be listed.

`AuctionHouseBot.RefreshLegacyListings` gradually expires unbid listings from
the older single Auctionbot GUID at startup. The production setting refreshes
400 per restart, which the normal seller then replaces under random character
names. Auctions with player bids are never selected for this refresh.

On 2026-10-29 the remaining 4,788 unbid, single-owner Auctionbot listings were
backed up and deleted while worldserver was stopped. The seller refilled the
market to 5,725 listings under 879 random bot names. No Auctionbot-owned or
bot-owned required-level-100 listings remained. The backup is kept locally in
`out/auctionbot-auctionhouse-backup-20261029-212733.sql` and is not committed.

The production `exports/worldserver.conf` enables this behavior alongside its
existing buyer and seller settings. Production database credentials and bot
account IDs are local configuration or database data and are not committed.

The local `exports/playerbots.conf` uses 80 random-bot accounts and a target of
80–100 online bots. Each account can hold multiple characters, so the market
can draw seller and buyer names from a much larger character pool. These are
simulated market actors; the online count controls actual world activity.

On the 2026-10-29 live test, the first cycle posted 40 auctions under 32
different RNDBOT characters. It expired 121 restricted legacy listings without
player bids. ExampleCharacter's low-priced Windwool Cloth listings then received bids
from several RNDBOT characters and several buyouts; sale mail rows were created
for ExampleCharacter. Client receipt after the mail delivery delay remains to be checked.
