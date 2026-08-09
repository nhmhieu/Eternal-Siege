#pragma once
#include "TextureManager.h"
inline bool preloadKingdomAssets(TextureManager& t) {
 bool ok=true;
 ok&=t.loadTexture("PlayerMage","assets/images/PlayerMage.png");
 ok&=t.loadTexture("PlayerMageWalk","assets/images/animations/PlayerMage_Walk.png");
 ok&=t.loadTexture("GuardWalk","assets/images/animations/Guard_Walk.png");
 ok&=t.loadTexture("CitizenWalk","assets/images/animations/Citizen_Walk.png");
 ok&=t.loadTexture("CastleGate","assets/images/kingdom/CastleGate.png");
 ok&=t.loadTexture("KingdomWorldOpen","assets/images/kingdom/kingdom_world_open.png");
 return ok;
}
