#include "combat.hpp"
#include <core/game/sdk.hpp>

auto powf_(float _X, float _Y) -> float { return (_mm_cvtss_f32(_mm_pow_ps(_mm_set_ss(_X), _mm_set_ss(_Y)))); };
auto GetCrossDistance(double x1, double y1, double x2, double y2) -> double { return sqrtf(powf((x2 - x1), 2) + powf_((y2 - y1), 2)); };
template<typename T> static inline T ImLerp(T a, T b, float t) { return (T)(a + (b - a) * t); }

auto SetMousePosition(FVector2D vTarget) -> void
{
	std::srand(static_cast<unsigned>(std::time(nullptr)));
	int iMaxMistakePx = 0;

	if (itx::settings.bHumanization)
		iMaxMistakePx = itx::settings.iMistakeSize;

	FVector2D vScreenCenter(itx::screen.fWidth / 2, itx::screen.fHeight / 2);
	FVector2D vDestination(0, 0);
	int iSmoothFactor = itx::settings.iSmooth + 1;

	if (vTarget.x != 0)
	{
		int xMistake = -iMaxMistakePx + (std::rand() % (2 * iMaxMistakePx + 1));
		vDestination.x = (vTarget.x > vScreenCenter.x) ? -(vScreenCenter.x - vTarget.x + xMistake) : (vTarget.x - vScreenCenter.x + xMistake);
		vDestination.x /= iSmoothFactor;
		vDestination.x = (vDestination.x + vScreenCenter.x > itx::screen.fWidth) ? 0 : vDestination.x;
	}

	if (vTarget.y != 0)
	{
		int yMistake = -iMaxMistakePx + (std::rand() % (2 * iMaxMistakePx + 1));
		vDestination.y = (vTarget.y > vScreenCenter.y) ? -(vScreenCenter.y - vTarget.y + yMistake) : (vTarget.y - vScreenCenter.y + yMistake);
		vDestination.y /= iSmoothFactor;
		vDestination.y = (vDestination.y + vScreenCenter.y > itx::screen.fHeight) ? 0 : vDestination.y;
	}

	if (itx::settings.bHumanization)
	{
		int iSleepTime = 1 + (std::rand() % 3);
		std::this_thread::sleep_for(std::chrono::milliseconds(iSleepTime));
	}

	//Set Cursor Position To vDestination
}

FVector2D vTargetScreen;
FVector vTargetBone;
__int64 pTargetBoneMap = 0;

inline bool bCanClick = false;
std::chrono::steady_clock::time_point tCachedTime;
bool bHasRecentlyClicked = false;
int iTimeSince;
std::chrono::steady_clock::time_point tpBeginning;
std::chrono::steady_clock::time_point tpEnding;
auto itx::combat_c::TriggerBot() -> void
{
	if (itx::settings.bTriggerbot)
	{
		auto tCurrentTime = std::chrono::steady_clock::now();
		auto fTimeElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(tCurrentTime - tCachedTime).count();

		if (fTimeElapsed > 250)
			bCanClick = true;

		int iBoneIndex = 0;
		switch (itx::settings.iHitBox)
		{
		case 0:
			iBoneIndex = 109;
			break;
		case 1:
			iBoneIndex = 67;
			break;
		case 2:
			iBoneIndex = 7;
			break;
		case 3:
			iBoneIndex = 2;
			break;
		case 4:
			iBoneIndex = 67;
			break;
		}

		float fDistance = Camera::Location.Distance(Cached::TargetPawn->GetRootComponent()->GetRelativeLocation()) / 100.f;
		bool bIsKeyDown = itx::settings.bIgnoreKeybind ? GetAsyncKeyState(itx::settings.iTriggerbotKeybind) : true;
		if (Cached::TargetPawn && (fDistance <= 100) && Cached::PlayerController->IsTargetUnderReticle(Cached::TargetPawn, pTargetBoneMap, iBoneIndex) && bIsKeyDown)
		{
			if (itx::settings.bShotgunOnly)
			{
				if (Cached::FortWeaponType == EFortWeaponType::Shotgun)
				{
					if (bHasRecentlyClicked)
					{
						tpBeginning = std::chrono::steady_clock::now();
						bHasRecentlyClicked = false;
					}

					tpEnding = std::chrono::steady_clock::now();
					iTimeSince = std::chrono::duration_cast<std::chrono::milliseconds>(tpEnding - tpBeginning).count();

					if (iTimeSince >= itx::settings.iCustomDelay)
					{
						if (bCanClick)
						{
							//Simulate Left Click
							bHasRecentlyClicked = true;
							bCanClick = false;
							tCachedTime = std::chrono::steady_clock::now();
						}
					}
				}
			}
			else
			{
				if (bHasRecentlyClicked)
				{
					tpBeginning = std::chrono::steady_clock::now();
					bHasRecentlyClicked = false;
				}

				tpEnding = std::chrono::steady_clock::now();
				iTimeSince = std::chrono::duration_cast<std::chrono::milliseconds>(tpEnding - tpBeginning).count();

				if (iTimeSince >= itx::settings.iCustomDelay)
				{
					if (bCanClick)
					{
						//Simulate Left Click
						bHasRecentlyClicked = true;
						bCanClick = false;
						tCachedTime = std::chrono::steady_clock::now();
					}
				}
			}
		}
	}
}

auto itx::combat_c::CombatThread() -> void
{
	for (;; )
	{
		int iBoneIDs[4] = { EBoneIndex::Head, EBoneIndex::Neck, EBoneIndex::Chest, EBoneIndex::Pelvis };
		vTargetScreen = { };
		vTargetBone = { };
		pTargetBoneMap = 0;

		auto tCachedTime = std::chrono::high_resolution_clock::now();

		for (;; )
		{
			auto tCurrentTime = std::chrono::high_resolution_clock::now();
			if (std::chrono::duration_cast<std::chrono::microseconds>(tCurrentTime - tCachedTime).count() < 5000) continue;
			tCachedTime = tCurrentTime;

			this->TriggerBot();

			if (Cached::Pawn)
			{
				Cached::TargetDistance = FLT_MAX;
				Cached::TargetPawn = NULL;

				for (auto& Actor : ActorList)
				{
					auto Pawn = Actor.Pawn;
					auto Mesh = Pawn->GetMesh();
					auto pBoneArray = Actor.pBoneArray;

					bool bIsVisible = false;

					if (itx::settings.bVisibleCheck)
						bIsVisible = Mesh->IsVisible();
					else
						bIsVisible = true;

					if (itx::settings.bVisibleCheck)
						if (!bIsVisible)
							continue;

					if (Pawn->IsDespawning())
						continue;

					if (Cached::TeamIndex == Actor.PlayerState->GetTeamIndex())
						continue;

					if (itx::settings.bAimbot)
					{
						FVector2D vHeadScreen = Cached::ViewState->WorldToScreen(Mesh->GetBoneMatrix(pBoneArray, 110));
						FVector2D vDistanceFromCrosshair = FVector2D(vHeadScreen.x - (itx::screen.fWidth / 2), vHeadScreen.y - (itx::screen.fHeight / 2));

						auto fDist = sqrtf(vDistanceFromCrosshair.x * vDistanceFromCrosshair.x + vDistanceFromCrosshair.y * vDistanceFromCrosshair.y);
						if (fDist < itx::settings.iFovRadius * 10 && fDist < Cached::TargetDistance)
						{
							Cached::TargetDistance = fDist;
							Cached::TargetPawn = Pawn;
							pTargetBoneMap = pBoneArray;
						}
					}
				}

				if (Cached::TargetPawn)
				{
					switch (itx::settings.iHitBox)
					{
					case 0:
						vTargetBone = Cached::TargetPawn->GetMesh()->GetBoneMatrix(pTargetBoneMap, 110);
						vTargetScreen = Cached::ViewState->WorldToScreen(vTargetBone);
						break;
					case 1:
						vTargetBone = Cached::TargetPawn->GetMesh()->GetBoneMatrix(pTargetBoneMap, 67);
						vTargetScreen = Cached::ViewState->WorldToScreen(vTargetBone);
						break;
					case 2:
						vTargetBone = Cached::TargetPawn->GetMesh()->GetBoneMatrix(pTargetBoneMap, 7);
						vTargetScreen = Cached::ViewState->WorldToScreen(vTargetBone);
						break;
					case 3:
						vTargetBone = Cached::TargetPawn->GetMesh()->GetBoneMatrix(pTargetBoneMap, 2);
						vTargetScreen = Cached::ViewState->WorldToScreen(vTargetBone);
						break;
					case 4:
						for (int i = 0; i < 3; i++)
						{
							auto vBoneWorld = Cached::TargetPawn->GetMesh()->GetBoneMatrix(pTargetBoneMap, iBoneIDs[i]);
							auto vBoneScreen = Cached::ViewState->WorldToScreen(vBoneWorld);
							if (vBoneScreen.Distance(FVector2D(itx::screen.fWidth / 2, itx::screen.fHeight / 2)) < vTargetScreen.Distance(FVector2D(itx::screen.fWidth / 2, itx::screen.fHeight / 2)))
							{
								vTargetBone = vBoneWorld;
								vTargetScreen = vBoneScreen;
							}
						}
						break;
					}
				}
			}

			if (Cached::TargetPawn && Cached::Pawn && GetAsyncKeyState(itx::settings.iAimbotKeybind) && !vTargetScreen.IsZero() && (GetCrossDistance(vTargetScreen.x, vTargetScreen.y, (itx::screen.fWidth / 2), (itx::screen.fHeight / 2) <= itx::settings.iFovRadius)))
			{
				auto CurrentWeapon = Cached::Pawn->GetCurrentWeapon();
				auto fTargetDistance = Camera::Location.Distance(vTargetBone);
				auto fProjectileSpeed = CurrentWeapon->GetProjectileSpeed();
				auto fProjectileGravity = CurrentWeapon->GetProjectileGravity();

				if (Cached::FortWeaponType != (EFortWeaponType::Melee || EFortWeaponType::Unarmed))
				{
					if (fProjectileSpeed && fProjectileGravity && CurrentWeapon && itx::settings.bPrediction)
					{
						auto pTargetRoot = itx::memory.read<uintptr_t>((uintptr_t)Cached::TargetPawn + Offset::RootComponent);
						auto vTargetVelocity = itx::memory.read<FVector>(pTargetRoot + Offset::ComponentVelocity);
						vTargetBone = vTargetBone.Predict(vTargetBone, vTargetVelocity, fTargetDistance, fProjectileSpeed, fProjectileGravity);
						vTargetScreen = Cached::ViewState->WorldToScreen(vTargetBone);
					}
				}

				if (!vTargetBone.IsZero())
					SetMousePosition(vTargetScreen);
			}
			else
			{
				vTargetBone = FVector(0, 0, 0);
				vTargetScreen = FVector2D(0, 0);
				Cached::TargetPawn = NULL;
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
		}
	}
}