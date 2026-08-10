#pragma once
class KingdomTransitionLatch{public:bool request(){if(requested)return false;requested=true;return true;}void reset(){requested=false;}bool isRequested()const{return requested;}private:bool requested=false;};
