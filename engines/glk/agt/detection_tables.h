/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "engines/game.h"
#include "common/gui_options.h"
#include "common/language.h"

namespace Glk {
namespace AGT {

const PlainGameDescriptor AGT_GAME_LIST[] = {
	{ "agt", "AGT IF Game" },

	{ "abloodylife", "A Bloody Life" },
	{ "alandria", "The Search for Princess Alandria" },
	{ "alchemistcastle", "Castle of the Alchemists" },
	{ "advalice", "The Adventures of Alice Who Went Through the Looking-Glass" },
	{ "apprenticetesting", "Apprentice - The Testing of a Magical Novice" },
	{ "sirarthur", "Sir Arthur" },
	{ "cercla", "Cercla" },
	{ "cardigan1", "Space Aliens Laughed at My Cardigan" },
	{ "cardigan2", "Still Laughing at my Cardigan" },
	{ "curse", "Curse of ManorLand" },
	{ "sanityclause", "Sanity Clause or, Why Santa Didn't Make It to YOUR House that Year" },
	{ "cliffdiver1", "Cliff Diver: Investigator for Hire - Case 1" },
	{ "cliffdiver2", "Cliff Diver: Investigator for Hire - Case 2" },
	{ "cosmoserve", "CosmoServe" },
	{ "crusade", "Crusade" },
	{ "agtdetective", "Detective" },
	{ "dragonschocolate", "Dragons in Chocolate Land" },
	{ "disenchanted", "Disenchanted" },
	{ "ducksoup", "Duck Soup" },
	{ "cavesofdyanty", "Caves of Dyanty" },
	{ "destinationearth", "Destination: Earth" },
	{ "dudleydilemma", "A Dudley Dilemma" },
	{ "80days", "Around the World in Eighty Days" },
	{ "easteregghunt", "Heather's Easter Egg Hunt" },
	{ "electrabot", "Electrabot" },
	{ "elf20", "The Elf's Christmas Adventure" },
	{ "elfquest", "Elf Quest" },
	{ "eliescape", "Escape from the ELI" },
	{ "emailbox", "E-MAILBOX" },
	{ "escapeprisonisland", "Escape from Prison Island" },
	{ "agtfable", "A Fable" },
	{ "firststupidgame", "My First Stupid Game" },
	{ "ccfirstadv", "Colossal Cave - The First Adventure" },
	{ "ggollek", "Ggollek I : The Dissolution" },
	{ "agtghosttown", "Ghost Town" },
	{ "giganticsecrets", "Secrets of the Gigantic" },
	{ "newenglandgothic", "New England Gothic" },
	{ "grailmisadventure", "The Misadventure of the Holy Grail" },
	{ "hardestadv", "The World's Hardest Adventure" },
	{ "helvera", "Helvera, Mistress of the Park" },
	{ "highe", "Highe, the Adventures of Elizabeth(\"El\") Highe" },
	{ "sirramichobbs", "Sir Ramic Hobbs and the High Level Gorilla" },
	{ "holmescasebook", "The Casebook of Sherlock Holmes" },
	{ "hotelnotell", "Hotel Notell" },
	{ "house2house", "House 2 House" },
	{ "agthugecave", "Adventure in Humongous Cave" },
	{ "hurryhurry", "Hurry!Hurry!Hurry!!" },
	{ "jackofhartz", "Jack of Hartz" },
	{ "jubileeroad", "Jubilee Road" },
	{ "killjustin", "Kill Justin" },
	{ "klaustrophobia1", "Klaustrophobia - Part I" },
	{ "klaustrophobia2", "Klaustrophobia - Part II" },
	{ "klaustrophobia3", "Klaustrophobia - Part III" },
	{ "klingonrpg", "In the Year 2366, Klingon Role Playing Game" },
	{ "deadlylabyrinth", "The Deadly Labyrinth" },
	{ "library", "Library - Library of Guilford College" },
	{ "lostgold", "Lost Gold" },
	{ "lostinspace", "Lost in Space : Dr.Smith Goes Home" },
	{ "agtlottery", "Lottery" },
	{ "loststonemansion", "Lost Stone Mansion" },
	{ "agtpyramids", "The Pyramids of Mars" },
	{ "mdthief", "The Multi-Dimensional Thief" },
	{ "agtmhpquest", "Quest for the Magic Healing Plant" },
	{ "mopandmurder", "Mop and Murder" },
	{ "agtmst3k1", "Detective, An Interactive MiSTing (Mystery Science Theater 3000)" },
	{ "agtmst3k2", "Mystery Science Theater 3000, Adventure 102" },
	{ "spacemule", "Space Mule" },
	{ "myopia", "Myopia" },
	{ "nmr1", "Adventures in NMR" },
	{ "nmr2", "Adventures in NMR II : The Adventure Continues" },
	{ "oceana", "Oceana" },
	{ "agtodieus", "Odieus's Quest for the Magic Flingshot" },
	{ "oklib", "Oklib's Revenge" },
	{ "ovanpelt", "Orientation to Van Pelt Library of the University of Pennsylvania" },
	{ "peterpatzer", "The Adventures of Peter Patzer" },
	{ "blackpearl", "Quest for the Black Pearl" },
	{ "battleofphilip", "The Battle of Philip against the Forces of Creation" },
	{ "flightintofantasy", "The Pilot or A Flight into Fantasy" },
	{ "pork1", "PORK I : The Great Underground Sewer System" },
	{ "pork2", "PORK II, The Gizzard of Showbiz" },
	{ "starportal", "The Star Portal" },
	{ "pastoralpitfalls", "Pastoral Pitfalls" },
	{ "personalizedsample", "Personalized Adventure Game Sample" },
	{ "lostproperty", "Lost Property" },
	{ "gameofrecovery", "The Game of Recovery" },
	{ "rerunsagain", "Reruns Again version" },
	{ "derring", "Der Ring des Nibelungen" },
	{ "sherwoodadv", "Adventures in Sherwood" },
	{ "shapeshifteradv", "Shape Shifter Adventure!" },
	{ "sirguygallant", "Sir Guy Gallant and the Deadly Warning" },
	{ "shadesofgray", "Shades of Gray" },
	{ "sonofstagefright", "Son of Stagefright" },
	{ "spatent", "The Spatent Obstruction" },
	{ "squynchia", "The Squynchia Adventure" },
	{ "agtstiffy", "The Incredible Erotic Adventures of Stiffy Makane!" },
	{ "storms1", "Storms I" },
	{ "susan", "Susan (A Lustful Game)" },
	{ "tamoret", "Tamoret" },
	{ "tarabithia", "Escape from Tarabithia" },
	{ "tarksimmons", "The Adventure of Tark Simmons" },
	{ "tarotia", "The Books of Tarotia : Book 1" },
	{ "tempest", "The Tempest" },
	{ "thegame", "Whatever We Decide To Call This Game" },
	{ "therift", "The Rift" },
	{ "tja", "The Jeweled Arena" },
	{ "toho", "Toho Academy" },
	{ "tombpharaohs", "The Tomb of the Ancient Pharaohs" },
	{ "tossedintospace", "Tossed into Space : Dr.Schmidt Goes Home" },
	{ "timesquared", "TimeSquared" },
	{ "folkestone", "Murder at the Folkestone Inn" },
	{ "void", "VOID:CORPORATION" },
	{ "wanderer1", "Black Wanderer 1 - The Darkest Road" },
	{ "wanderer2", "Black Wanderer 2 - The Unborn One" },
	{ "wanderer3", "Black Wanderer 3 - Twas a Time of Dread" },
	{ "weekendsurvival", "Weekend Survival" },
	{ "witchfinder", "Witchfinder" },
	{ "agtwizardscastle", "The Wizard's Castle" },
	{ "hobbswok", "Sir Ramic Hobbs and the Oriental Wok" },
	{ "wraithblaster", "Wraith Blaster" },
	{ "journeyintoxanth", "A Journey into Xanth" },
	{ "zanfar", "Zanfar" },

	// Dutch games
	{ "querido", "Querido" },

	{ nullptr, nullptr }
};

const GlkDetectionEntry AGT_GAMES[] = {
	DT_ENTRY0("abloodylife", "c492e0ae0647d3a4835012ca864b99d5", 157221),
	DT_ENTRY0("alandria", "0dcaff32c55dd2c1898da7893500de34", 53946),
	DT_ENTRY0("alchemistcastle", "7822dfaf1ae31b3e508e7b0a267d054b", 192051),
	DT_ENTRY0("advalice", "0aaafb897b46baa28023bbbaf4091fd8", 23004),
	DT_ENTRY0("apprenticetesting", "4e4244649dc1cd39546f3d10dc85acb5", 131868),
	DT_ENTRY0("sirarthur", "46956e2d28f6b926fc6831d60f891ffc", 120204),
	DT_ENTRY0("cardigan1", "301509b196fd27c87d5d176f895b94ea", 103356),
	DT_ENTRY0("cardigan2", "f17a9d5401cb5cb1be4cb2719d0c9d34", 97767),
	DT_ENTRY0("cercla", "a56219015b70f507d9a1f74e0a92db1f", 136080),
	DT_ENTRY0("curse", "b09a74de6081e4d56e0348c9951623e9", 79139),
	DT_ENTRY0("sanityclause", "a7ea1c9ae6200511af71dfcebb5d55ff", 246159),
	DT_ENTRY0("cliffdiver1", "14ce6a122a061f2b361e725fe2c0c0e4", 120042),
	DT_ENTRY0("cliffdiver2", "9cc68e22a0ba03fe13bd4bfb413e08df", 155682),
	DT_ENTRY1("cosmoserve", "Final", "fce21feb3a6dfda1298d3eb3b46ef0b2", 377460),
	DT_ENTRY0("cosmoserve", "e677a308c446af4e076a26ef0ca235ad", 365229),
	DT_ENTRY0("crusade", "d7df6bc394d225ab023e4f099d982156", 50463),
	DT_ENTRY0("agtdetective", "b17f780a90fa4e0e30e5bbf590f78cd5", 17901),
	DT_ENTRY0("dragonschocolate", "6cb0714d337ed45ae03e6a54ed60fdc4", 143208),
	DT_ENTRY0("disenchanted", "7003a85672bbfa067dc6a28a295a1ad1", 99630),
	DT_ENTRY0("ducksoup", "e3c609c2a78e89b03c8cdefa19a50293", 83187),
	DT_ENTRY1("dudleydilemma", "1.2", "2ff4de040b7cee9592bc8dc2e020d937", 111294),
	DT_ENTRY1("dudleydilemma", "3.0", "4cdea9d3acc19f9a02072517e4bc463d", 190188),
	DT_ENTRY0("cavesofdyanty", "267e8a2812d58e140be8582914d9cefb", 40662),
	DT_ENTRY0("destinationearth", "d00cfa53e2b3315f0ee6813c064be74f", 12474),
	DT_ENTRY0("80days", "0086c0151760c59aa4d9e8ca055de84d", 30294),
	DT_ENTRY0("easteregghunt", "6ef9fb84ec755b88f1f7c2cc3c47db2e", 55647),
	DT_ENTRY0("electrabot", "1c7096e4a9a0579526e9b5084aa27701", 8748),
	DT_ENTRY0("elf20", "0fa1e888a452fec59bb4a5a6ffa43d78", 101088),
	DT_ENTRY0("elfquest", "5419ab5d7a19037a5971c7e2de59cee4", 16929),
	DT_ENTRY0("eliescape", "8d604abcccccbc0064b7488497f6242d", 72414),
	DT_ENTRY0("emailbox", "f90b34f0f2d7dfb3c7f29fbae9897671", 55908),
	DT_ENTRY0("escapeprisonisland", "8f6cf9b1f46e968b353bd00a48c2bd6b", 48762),
	DT_ENTRY0("agtfable", "9acb005ddd793da7898eda2bbc79a9d3", 15147),
	DT_ENTRY0("ccfirstadv", "8a8ff26cd6a396c193d865fa6e37594d", 83754),
	DT_ENTRY0("firststupidgame", "859933f151a301f64f88a8101853f432", 21222),
	DT_ENTRY0("ggollek", "e02fa5e1ddff57e89231481574218834", 75573),
	DT_ENTRY0("agtghosttown", "33aa534de04a978c50f8a038a6bec3e7", 35235),
	DT_ENTRY0("giganticsecrets", "66d6b6b5bf43149a8ad5578c45ad4731", 21627),
	DT_ENTRY0("newenglandgothic", "10898900c3b872282a4298b32e851dfc", 104895),
	DT_ENTRY0("grailmisadventure", "f7b0447cc01d1f4629e734952deccf98", 107487),
	DT_ENTRY0("hardestadv", "326aaac9509503829e2b350b867c4ca5", 115263),
	DT_ENTRY0("helvera", "aa1ba7a1f1726a90eec90b0eb998cce8", 104642),
	DT_ENTRY0("highe", "8c08f8e0e215d1293398b0d652578baf", 15471),
	DT_ENTRY0("sirramichobbs", "ba008ad6016d8302dd4311dd20ccb4e0", 132597),
	DT_ENTRY0("holmescasebook", "391e0bd51cbf8bc4cfffe751a1a659b2", 256446),
	DT_ENTRY0("hotelnotell", "0c54347ebbcfe32bbf143a1574cdb445", 111132),
	DT_ENTRY0("house2house", "9e5ee1005108afc247063e5f770ab1cc", 78246),
	DT_ENTRY0("agthugecave", "0364693bb31fb2e9a45927f9e542b1fa", 260415),
	DT_ENTRY0("hurryhurry", "040ca0ed40cb4732b66c2ab9b68bca97", 165564),
	DT_ENTRY0("jackofhartz", "74d754d8ce9bb7dca6f70b60c72ee27d", 97038),
	DT_ENTRY0("klingonrpg", "93811c560f0c78f470f65dbe62834aa1", 15066),
	DT_ENTRY0("deadlylabyrinth", "3a5d3ad2f80fb8c02baf5eb9894eb9b6", 113643),
	DT_ENTRY0("library", "f23d106273f6e5fdb50f65d2acd4e4fc", 133407),
	DT_ENTRY0("lostgold", "ff08d607b3a1a787b5d9e369264ae7f8", 67959),
	DT_ENTRY0("lostinspace", "322c226f26768b6962c2b3b147450410", 49410),
	DT_ENTRY0("agtlottery", "7c0890c420d6585e4629f1cc228bf259", 24948),
	DT_ENTRY0("loststonemansion", "f0ef6d965533e67b29acb130dd0f1213", 39933),
	DT_ENTRY0("jubileeroad", "f24fef5bc936c22fbd84c0929d727cbf", 105543),
	DT_ENTRY0("killjustin", "94d50b925733e70cf39079a8816b199c", 65043),
	DT_ENTRY0("klaustrophobia1", "cbcc82df28e67d89399139e5f234d8fc", 242838),
	DT_ENTRY0("klaustrophobia2", "b535015af4fece71c9f120730cb453dc", 292329),
	DT_ENTRY0("klaustrophobia3", "47aad0cb89ebe10e54172db55124b8d1", 366039),
	DT_ENTRY0("mdthief", "e62d36630c8a301a5da4192dfd28d650", 243729),
	DT_ENTRY0("agtpyramids", "cb2aa53dea87209fee2e300cd5396e4a", 126522),
	DT_ENTRY0("mopandmurder", "23c4a7ee63dbfb78871b7040a011cd89", 86913),
	DT_ENTRY0("agtmhpquest", "5d657aac27f1dc150d74c50251584af0", 29646),
	DT_ENTRY0("agtmst3k1", "53552013cadf6b62a5c8dcbb7f2af4a8", 127737),
	DT_ENTRY0("agtmst3k2", "973cf89bf1cea65ebd8df72c6d01354d", 107001),
	DT_ENTRY0("spacemule", "96cc0630552bc6a343e022777b40d9fd", 79056),
	DT_ENTRY0("myopia", "b3f3d0ae4fe3bf1181fa437c69b90016", 69859),
	DT_ENTRY0("nmr1", "c1758cd84fceade19866007f8d7c397f", 49734),
	DT_ENTRY0("nmr2", "979ffa08dc3b102b59f6893e4a4dede9", 55485),
	DT_ENTRY0("oceana", "63a163d87abf793a5e5c2f98f0d4c469", 178200),
	DT_ENTRY0("agtodieus", "aef479600d4fb82f8eedbeda855a9706", 28512),
	DT_ENTRY0("oklib", "d833679f11041ab1155b5207aabfc873", 166374),
	DT_ENTRY0("ovanpelt", "60a49ce4b7f99968cf92ccef5ad403f7", 53298),
	DT_ENTRY0("peterpatzer", "6a1be7e416f66c54b22e1305165fd7ee", 62842),
	DT_ENTRY0("blackpearl", "12419db6d6088e66394ecf5f28baa68d", 80109),
	DT_ENTRY0("battleofphilip", "8bbfd3d06b9eb4df0565e158e41312d8", 97443),
	DT_ENTRY0("flightintofantasy", "063f4f434b64c25f2ca816a564edbe35", 100521),
	DT_ENTRY0("pork1", "389deffc77cc58cce1ad8c0c57a5cfa8", 105948),
	DT_ENTRY0("pork2", "13911c59cbe70ae877c87aa0ded89e47", 28269),
	DT_ENTRY0("starportal", "0bf0f86fdeea607083c22a5cb41c6885", 172935),
	DT_ENTRY0("pastoralpitfalls", "c35d440286c6bf67cd6ee1e5947c3483", 206469),
	DT_ENTRY0("personalizedsample", "c590a3c5116ee2fa786e8f511ef85c8e", 69174),
	DT_ENTRY0("lostproperty", "8acf3d6994a3b39911827d5040e8873a", 30375),
	DT_ENTRY0("gameofrecovery", "b497bb0e1e93023a892f0fa54d78a1c0", 108459),
	DT_ENTRY0("rerunsagain", "d263341c871a2f52e0052c313bf3e525", 81648),
	DT_ENTRY0("derring", "5553e1a6966525da7ab2d874090d3758", 52893),
	DT_ENTRY0("sherwoodadv", "270be7ce551c615d4c34bc64acd4c190", 313551),
	DT_ENTRY0("shapeshifteradv", "8a45a92074747edf8882ea2eaf6cfa54", 137862),
	DT_ENTRY0("sirguygallant", "c4376d121b26bc691b6a43b9f77eb22a", 125698),
	DT_ENTRY1("shadesofgray", "Final", "e93ed21cdafc1b998ee2ccab557f0649", 433350),
	DT_ENTRY0("shadesofgray", "677753739047deb5ccf72f1b6555c677", 431568),
	DT_ENTRY0("sonofstagefright", "9527fa27e910470deac8ffbcb29e2427", 116640),
	DT_ENTRY0("spatent", "acc4c60cbb9d0239ab9b1900b239771a", 85455),
	DT_ENTRY0("squynchia", "e9e5c99ee87f3b38a9ea8e7fdd1ed79f", 81000),
	DT_ENTRY0("agtstiffy", "a7f1902ab7aa9972ca46d5b36d06d2b1", 32805),
	DT_ENTRY0("storms1", "8567c2db37c80f015a950ef80d299a0a", 111942),
	DT_ENTRY0("susan", "cb71705848aabcac90e7ea9e911ceee9", 15633),
	DT_ENTRY0("tamoret", "3de37497ed763a58093e556a963ca14e", 156816),
	DT_ENTRY0("tarabithia", "6734a6889d825dae528d2a7efaf6dee2", 83430),
	DT_ENTRY0("tarksimmons", "cf6945fc43e8a3062a27fc39e01c3d6e", 116397),
	DT_ENTRY0("tarotia", "fbeac90159dc88e82240b4201b6231d5", 61479),
	DT_ENTRY0("tempest", "114b5224e7bb8def06a87c3910d7c4f3", 52650),
	DT_ENTRY0("thegame", "af6e39aadf8dced6f29d03650125a6d6", 139968),
	DT_ENTRY0("therift", "1c30da9b9a55d691226c45b8b53c11c3", 41877),
	DT_ENTRY0("tja", "6699e867df8263dd7858d2f6fb84acde", 517185),
	DT_ENTRY0("toho", "58a6fdf89b29966774beaca80f505fff", 228744),
	DT_ENTRY0("tombpharaohs", "2d10501417f28ee1dc5be8479f6e88a3", 46251),
	DT_ENTRY0("tossedintospace", "515f06782c5b11108a719a20e182166c", 49491),
	DT_ENTRY0("timesquared", "55e36771d5e1fe184cce8f5be666ff9f", 105300),
	DT_ENTRY0("folkestone", "7e949a7376b0a64cee0d9412b0203611", 64557),
	DT_ENTRY0("wanderer1", "e1d707c9deaf02a4b28c9041a4009cb6", 53946),
	DT_ENTRY0("wanderer2", "89dd16629022c75f3ffc171a6b126da6", 46980),
	DT_ENTRY0("wanderer3", "839ab34bce5c82ec6194675f0186b15b", 45765),
	DT_ENTRY0("weekendsurvival", "e770c0e75b7257eae9d4677340beca10", 91044),
	DT_ENTRY0("witchfinder", "9acecd1803d2e99282970db1ef6ff344", 186300),
	DT_ENTRY0("agtwizardscastle", "3adecad94b61babdadfbe20242e86b24", 18792),
	DT_ENTRY0("hobbswok", "3178e271e8259a889df99545d6c65362", 198369),
	DT_ENTRY0("wraithblaster", "392f507d42c006a30c55a20ec9e75f44", 194643),
	DT_ENTRY0("void", "b6818cc6396e1357c3c551bc338c653d", 53784),
	DT_ENTRY0("journeyintoxanth", "2b073d48a8a01f91d7bca5db482e3ecd", 147177),
	DT_ENTRY0("zanfar", "5fc6914fe02c0235f8a5343db8b6359e", 83106),

	// Dutch games
	DT_ENTRYL0("querido", Common::NL_NLD, "e52fe3a44d7b511bb362ce08a48435ef", 104166),

	DT_END_MARKER
};

} // End of namespace AGT
} // End of namespace Glk
