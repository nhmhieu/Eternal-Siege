#pragma once
#include "DayNightSystem.h"
#include "KingdomNpc.h"
enum class NpcActivity{Market,Fountain,Patrol,GuardPost,Home,Rest};
class NpcScheduleSystem{
public:
 static NpcActivity activity(KingdomNpcRole role,DayPhase phase){
  if(role==KingdomNpcRole::Guard)return phase==DayPhase::Night?NpcActivity::GuardPost:NpcActivity::Patrol;
  if(phase==DayPhase::Day)return NpcActivity::Market;if(phase==DayPhase::Dawn)return NpcActivity::Fountain;if(phase==DayPhase::Sunset)return NpcActivity::Home;return NpcActivity::Rest;
 }
};
