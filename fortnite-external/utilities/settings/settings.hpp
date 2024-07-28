#include <dependencies/framework/imgui.h>

namespace itx
{
	class settings_c
	{
	public:
		bool bAimbot = true;
		
		bool bPrediction = true;
		bool bVisibleCheck = true;
		bool IgnoreDowned = true;
		bool bRenderFOV = true;
		int iFovRadius = 15;
		int iSmooth = 5;
		int iHitBox = 1;
		int iAimbotKeybind = VK_LCONTROL;

		bool bTriggerbot = true;
		bool bShotgunOnly = true;
		bool bIgnoreKeybind = false;
		int iCustomDelay = 100;
		int iMaxDistance = 15;
		int iTriggerbotKeybind = VK_LCONTROL;

		bool bHumanization = true;
		int iMistakeSize = 10;
		int iMistakeCorrection = 10;

		bool bUsername = true;
		bool bDistance = true;
		bool bHeldWeapon = true;
		bool bSkeletons = false;
		bool bBox = true;
		int iBoxType = 0;
		bool bBoxOutline = true;
		bool bTextOutline = true;
		int iSkeletonThickness = 2;
		int iBoxThickness = 2;
		int iRenderDistance = 280;

		bool bvSync = true;
		bool bStreamProof = false;
		bool bStreamProofFlip = false;

		ImColor iVisibleColor = ImColor( 123, 186, 150 );
		ImColor iInvisibleColor = ImColor( 186, 123, 123 );
	};
	inline settings_c settings;
}