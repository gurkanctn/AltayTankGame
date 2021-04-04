
#include "pch.h"
#define OLC_PGE_APPLICATION

#include "olcPixelGameEngine.h"
#include "olcPGEX_Graphics2D.h"
#include "olcPGEX_Sound.h"

using namespace std;

const float PI = 3.1415926535f; // it could as well be 3.142

/* 
ALTAY TANK GAME
G.�etin, 2018-2020

See https://github.com/gurkanctn/AltayTankGame for issues, pull request and other info.

Use as-is. No guarantee is given.

The game works on Windows 10, 64bit. or Linux.
Feel free to port the game to Linux, Android, or other OS.

The licences of third party libraries are available at: https://github.com/gurkanctn/AltayTankGame
*/


class Altay : public olc::PixelGameEngine
{
public:
	Altay()
	{
		sAppName = "ALTAY TANK GAME, v2.0a"; //First Linux version, based on 1.5.1
	}

private:
	float fGlobalTime = 0.0f;
	float fTimerPause = 0.0f;
	float fTimerText = 0.0f;
	float fWorldX = 0.0f;
	float fWorldY = 0.0f;
	int gameScore;	
	int nGameState;		//splash screen,  game, endGame
	int prevGameState;
	bool AssetsLoaded = false;
	size_t nDisplayTextPlace = 0;
	size_t nDisplayTextStart = 0;
	float fTextDisplayDuration = 0.0f;
	bool bTypeTextDone;
	bool bGamePaused = false;
	int nLevel;
	int HiScore = -10;
	float RemainingPowerUp = 0;
	string BestPlayerName[3];
	bool bStateEntry = true;
	olc::Sprite *buffBack;
	olc::Sprite *sprBackground;				
	olc::Sprite *sprSplashScreen;
	olc::Sprite *sprPowerUp_01;
	olc::Sprite *sprEnemy1;
	olc::Sprite *sprEnemy2;

	int sndSplashScreen;
	int sndGameOver;
	int sndGameOverOnce;
	int sndGameBackground;
	int sndFireCannon;
	int sndExplode;
	int sndPowerUp1;
	int sndAutopilotOn;
	int sndAutopilotOff;
	int nTest;
	string sStory;

	struct sPlayer {
		float px = 0.0f;
		float py = 0.0f;
		float vx = 0.0f; // speed in X direction
		float vy = 0.0f; // speed in Y direction
		float health = 1000.0f;
		float speed = 0.80f;
	};// Define player variables
	sPlayer Player;
	float fFireRate = 0.25f;

	float fFireRateAcc = 0.0f;
	float fCannonTemperature = 0.0f;
	float FireSpeed = 0.0f;
	float maxSpeed = 2.0f;		//max speed in U and V
	float fTimePowerUp = 0.0f;
	float fHeading;
	float fBearing = 0.0f;
	float fBearingNext = 0.0f;
	olc::Sprite *sprPlayer;		//tank_body of the player
	olc::Sprite *sprTurret;
	
	struct sEnemy {
		float px;
		float py;
		float vx;
		float vy;
		float hdg;	//heading
		int health;
		float firePower;
		float fireSpeed;
		float fireRate;
		float fireRateAcc;
		float speedMax;
		olc::Sprite *sprEnemy;
	};//Define Enemy variables
	list<sEnemy> listEnemies;

	struct sBullet{
		float px;
		float py;
		float vx;
		float vy;
		float hitPoint;
		float speed;
		float speedFactor;
		bool hasExploded;
		bool bEnemyFire;
	};
	list<sBullet> listBullets;

	struct sExplosion {
		float px;
		float py;
		float sizeMax;
		float currentSize;
	};
	list<sExplosion> listExplosions;
	
	struct sPowerup {
		float px;
		float py;
		float vx;
		float vy;
		float fPlayerFireRate;
		float fBulletHitPoint;
		float fBulletSpeedFactor;
		float fPlayerHealth;
		float fScore;
		float fPlayerMaxSpeed;
		float timer;
		float timeout;
		olc::Sprite *sprPowerUp;
	};
	list<sPowerup> listPowerups;
	
	struct sActivePowerup {
		float timer;
		float timeout;
	};
	list<sActivePowerup> listActivePowerups;
	

	struct sWP{
		int id;
		float x;
		float y;
	};
	list<sWP> listPath;
	
	sWP tempWP;
	sWP targetWP;
	bool bAutoNav = false;
	int targetWPid = 1;

	float tunerX = 1.0;		//autopilot X gain
	float tunerY = 1.0;     //autopilot Y gain


	string readStory(string filename)                                            // @mbozzi / @Thomas1965 : via stringstream buffer
	{
		stringstream buffer;
		buffer << ifstream(filename).rdbuf();
		return buffer.str();
	}
	void TypeStory(string text, float fElapsedTime) {
		fTimerText += fElapsedTime;
		
		bTypeTextDone = false;
		size_t i = nDisplayTextPlace;
		size_t j = nDisplayTextStart;
		float fDuration = 0.0f;
		switch (text[i]) {
			case '\n':
				fDuration = 1.0f;
				break;
			default:
				fDuration = 0.15f * std::rand() / RAND_MAX;
		}
			
		if (text[i] == '\n') {
			nDisplayTextStart = i;
			//fTimerText = fElapsedTime * 0.9;
		}

		if (fTimerText > fDuration) {
			fTimerText = 0.0f;
			nDisplayTextPlace++;
			i = nDisplayTextPlace;
		}
		//char newLine = '\n';
		if (i <= text.size()) {
			//cout << j << " - " << i << endl;	
			DrawString(0 + 2, int(ScreenHeight() * 4.0f / 5.0f + 2.0f), text.substr(j, i-j), olc::DARK_GREY, 2);
			DrawString(0, int(ScreenHeight() * 4.0f / 5.0f), text.substr(j, i-j), olc::WHITE, 2);
		}
		if (GetKey(olc::Key::SPACE).bHeld) i = int(text.size());
		if ((i == text.size()) || (bTypeTextDone)) {
			nDisplayTextPlace = int(text.size());
			fTextDisplayDuration += fElapsedTime;
			bTypeTextDone = true;
		}
	}
	
	void setTargetWP(int WPID) {
		for (auto &WP : listPath)
		{
			if ((WP.id) == targetWPid) {
				targetWP = WP;
			}
		}
	}

	void updateWP(int WPID, float posX, float posY) {
		for (auto &WP : listPath)
		{
			if ((WP.id) == WPID) {
				WP.x = posX;
				WP.y = posY;
			}
		}
	}

	float wrap_PI(const float angle)
	{
		float res = fmodf(angle, PI);
		if (res < 0) {		//replace with "res += (res < 0) * PI;"
			res += PI;
		}
		return res;
	}

	float wrap_2PI(const float angle)
	{
		float res = fmodf(angle, 2*PI);
		if (res < 0) {		//replace with "res += (res < 0) * 2* PI;"
			res += 2*PI;
		}
		return res;
	}


	float distance(const float x1, const float y1, const float x2, const float y2) {
		return (x2 - x1) * (x2-x1) + (y2 - y1) * (y2 - y1);
	}

	void SplashScreen(float fElapsedTime) {
		fGlobalTime += fElapsedTime;
		//std::srand(std::time(nullptr));
		SetDrawTarget(buffBack);
		olc::GFX2D::Transform2D t;
		t.Scale(1.8f, 2.2f);
		t.Translate(-160.0f, 0.0f);
		olc::GFX2D::DrawSprite(sprSplashScreen, t);
		//wait for space key to start game
		DrawString(ScreenWidth() / 12,			ScreenHeight() / 10,		"ALTAY TANK", olc::DARK_GREY, 10);
		DrawString(ScreenWidth() / 12 - 4,		ScreenHeight() / 10 - 4,	"ALTAY TANK", olc::GREEN, 10);
		DrawString(int(ScreenWidth() * 0.25f + sin(fGlobalTime) * 10.0f), int(ScreenHeight() * 0.75f + 4.0f * cos(fGlobalTime)),		"PRESS SPACE TO START...", olc::VERY_DARK_GREY, 3);
		DrawString(int(ScreenWidth() * 0.25f - 4 + sin(fGlobalTime) * 10.0f), int(ScreenHeight() * 0.75f - 4.0f + 4.0f * cos(fGlobalTime)), "PRESS SPACE TO START...", olc::RED, 3);

		//TYPE STORY TEXT TO SCREEN
		if (fTextDisplayDuration < 5.0f) TypeStory(sStory, fElapsedTime); //(!bTypeTextDone) || 
		if (fTextDisplayDuration >= 5.0f) {
			nDisplayTextPlace = 0;
			nDisplayTextStart = 0;
		}
		//Draw to Screen now
		SetDrawTarget(nullptr);
		DrawSprite(0, 0, buffBack);

		//todo: run function "INIT();"
	}
	void PlayGame(float fElapsedTime) {
		int nMouseX = GetMouseX();
		int nMouseY = GetMouseY();
		int nMouseWheel = GetMouseWheel();

		//sWP targetWP = tempWP;

		//update Player
		float dx = (float)nMouseX - Player.px + fWorldX;
		float dy = (float)nMouseY - Player.py + fWorldY;
		fHeading = atan2(dy, dx);
		fBearingNext = atan2(Player.vy, Player.vx) - PI; //
		
		//turn towards the closer direction
		//not working

		fBearingNext = wrap_2PI(fBearingNext);	//target bearing
		fBearing = fBearingNext;
												/*
		fBearing = wrap_2PI(fBearing);
		cout << fBearing << " --> " << fBearingNext << endl;
		//		float modulo = fmodf(fBearingNext - fBearing + PI, 2*PI);

		if ((fBearingNext - fBearing) < 0) { //yaw CLOCKWISE 
			fBearing += ((fBearingNext - fBearing)) * 0.25f;
		}
		else { // yaw COUNTERCLOCKWISE
			fBearing += (fBearingNext - fBearing) * 0.25f;
		}
			
			//cout << fBearing << " --> " << fBearingNext << endl;
		
		//fBearing = fBearing + (fBearingNext - fBearing) * 0.25;

		*/

		if (GetKey(olc::Key::A).bHeld) Player.vx -= Player.speed * fElapsedTime; //accelerate LEFT
		if (GetKey(olc::Key::D).bHeld) Player.vx += Player.speed * fElapsedTime; //accelerate RIGHT
		if (GetKey(olc::Key::W).bHeld) Player.vy -= Player.speed * fElapsedTime; //accelerate UP
		if (GetKey(olc::Key::S).bHeld) Player.vy += Player.speed * fElapsedTime; //accelerate DOWN
		if (GetKey(olc::Key::ESCAPE).bHeld) bGamePaused = true;
		if (GetKey(olc::Key::TAB).bReleased) {
			if (bAutoNav == false) {
				olc::SOUND::PlaySample(sndAutopilotOn);
				bAutoNav = true;
			}
			else {
				olc::SOUND::PlaySample(sndAutopilotOff);
				bAutoNav = false;
			}

		}
		/*
		if (GetKey(olc::Key::Q).bHeld) {
			olc::SOUND::PlaySample(nTest%6);
		}
		if (GetKey(olc::Key::E).bHeld) {
			nTest = (nTest+1)%3;
			if (nTest == 0) olc::SOUND::PlaySample(sndFireCannon);
			else if (nTest == 1) olc::SOUND::PlaySample(sndExplode);
			else if (nTest == 2) olc::SOUND::PlaySample(sndPowerUp1);
		}
		*/
		if (GetKey(olc::Key::K1).bHeld) targetWPid = 1;
		if (GetKey(olc::Key::K2).bHeld) targetWPid = 2;
		if (GetKey(olc::Key::K3).bHeld) targetWPid = 3;
		if (GetKey(olc::Key::K4).bHeld) targetWPid = 4; 
		if (GetKey(olc::Key::K5).bHeld) nLevel = 5; //trick for debug

		setTargetWP(targetWPid);

		// set Autopilot Gains using ERTY keys
		if (GetKey(olc::Key::E).bHeld) { // Increase TunerX
			tunerX++; // = min(1.1 * tunerX, 100.0);
		}
		if (GetKey(olc::Key::R).bHeld) { // Decrease TunerX
			tunerX--; // = max(0.9 * tunerX, 0.5);
		}
		if (GetKey(olc::Key::T).bHeld) { // Increase TunerX
			tunerY++; // = min(1.1 * tunerY, 100.0);
		}
		if (GetKey(olc::Key::Y).bHeld) { // Decrease TunerX
			tunerY--;// = max(0.9 * tunerY, 0.5);
		}

		// navigate to targetWP, using simple Proportional gain, update TargetWP if arrived
		if (bAutoNav) {
			if ((distance(targetWP.x, targetWP.y, Player.px, Player.py) < 250)) {
				targetWPid++;
				if (targetWPid > 4) targetWPid = 1; //4 = nMaxWPID
				setTargetWP(targetWPid);
			}
			Player.vx = (targetWP.x - Player.px);
			Player.vy = (targetWP.y - Player.py);
		}
		
		Player.vx = 0.99f * Player.vx;
		Player.vy = 0.99f * Player.vy;

		Player.vx = min(Player.vx, maxSpeed);
		Player.vx = max(Player.vx, -maxSpeed);
		Player.vy = min(Player.vy, maxSpeed);
		Player.vy = max(Player.vy, -maxSpeed);

		Player.px += Player.vx;
		Player.py += Player.vy;

		//todo: handle fWorldX and fWorldY here, to move along the map.
		Player.px = min(Player.px, (float)ScreenWidth());
		Player.px = max(Player.px, 0.0f);
		Player.py = min(Player.py, (float)ScreenHeight());
		Player.py = max(Player.py, 0.0f);

		//Player.health += fElapsedTime;
		Player.health = min(Player.health, (float)1000);

		//gameScore += fElapsedTime * 100;
		if (gameScore > pow(nLevel, 1.2) * 10000) {
			nLevel++;
			Player.health += 100;
			Player.health = min(Player.health, (float)1000);
			maxSpeed = maxSpeed * 1.025f;
			Player.speed = Player.speed * 1.025f;
		}

		if (listEnemies.size() < (0.3 * nLevel +3) ) {
			//Generate one Enemy (approx every 2 seconds (1/100 chance)
			int dicer = 0;

			dicer = std::rand() % (500) / (nLevel + 3);	//change difficulty according to player Health and LEVEL!
			if (GetKey(olc::Key::E).bHeld) {
				dicer = 0;
			}
			if (GetKey(olc::Key::R).bHeld) {
				dicer = 1;
			}
			if (dicer == 0 && nLevel > 3) {
				sEnemy e;
				e.px = Player.px;
				e.py = Player.py;

				while (( distance(Player.px, Player.py, e.px, e.py)) < 20000) {
					e.px = float(std::rand() / (RAND_MAX / ScreenWidth()));
					e.py = float(std::rand() / (RAND_MAX / ScreenHeight()));
				}
				e.sprEnemy = sprEnemy1;
				e.vx = 10.0f * (std::rand() % 20 - 10);
				e.vy = 10.0f * (std::rand() % 20 - 10);
				e.health = 150;		//full health
				float dxE = (float)e.px - Player.px;	// enemy should face Player
				float dyE = (float)e.py - Player.py;
				float fEnemyHeading = atan2(dyE, dxE) - 1.5708f;	//rotate 90 deg to match with image
				e.hdg = fEnemyHeading + float((std::rand() / RAND_MAX - 0.5f)*0.3145f);
				e.firePower = 0.5 * (nLevel > 2) ? 10.0f : 0.0f;
				e.fireRate = 1.2f;
				e.fireRateAcc = 0.0f;
				e.fireSpeed = 2.0f;
				e.speedMax = 20.0f + (nLevel*1.25f);
				listEnemies.push_back(e);
			}
			else if (dicer == 1) {
				sEnemy e;
				e.px = Player.px;
				e.py = Player.py;
								
				while (distance(Player.px, Player.py, e.px, e.py) < 20000) {
					e.px = float(std::rand() / (RAND_MAX / ScreenWidth()));
					e.py = float(std::rand() / (RAND_MAX / ScreenHeight()));
				}
				e.sprEnemy = sprEnemy2;
				e.vx = 10.0f * (std::rand() % 20 - 10);
				e.vy = 10.0f * (std::rand() % 20 - 10);
				e.health = 40;		//full health
				float dxE = (float)e.px - Player.px;
				float dyE = (float)e.py - Player.py;
				float fEnemyHeading = atan2(dyE, dxE) - 1.5708f;	//rotate 90 deg to match with image
				e.hdg = fEnemyHeading + (std::rand() / RAND_MAX - 0.5f)*0.3145f;
				e.firePower = 0.5*(nLevel > 2) ? 10 : 0.0f;
				e.fireRate = 1.5f;
				e.fireRateAcc = 0.0f;
				e.fireSpeed = 2.0f;
				e.speedMax = 10.0f + (nLevel*1.25f);
				listEnemies.push_back(e);
			}
			else if (abs(dicer-10) < 1 ) {
				sPowerup pu;
				pu.sprPowerUp = sprPowerUp_01;
				pu.px = float(std::rand() / (RAND_MAX / ScreenWidth())); //+ScreenWidth();
				pu.py = float(std::rand() / (RAND_MAX / ScreenHeight())); //+ScreenHeight();
				pu.vx = 0.0f;  //10 * (std::rand() % 20 - 10);
				pu.vy = 0.0f;  //10 * (std::rand() % 20 - 10);
				pu.fBulletHitPoint = 0;
				pu.fBulletSpeedFactor = 0.95f;
				pu.fPlayerFireRate = 0.85f;
				pu.fPlayerHealth = 20.0f;
				pu.fScore = 150;
				pu.fPlayerMaxSpeed = 4.0;
				pu.timeout = 15.0f;
				pu.timer = 0.0f;
				listPowerups.push_back(pu);

			}
			else
			{
				//do not spawn any enemies
			}
		}

		//first click immediate fire
		if (fFireRateAcc < fFireRate) fFireRateAcc += fElapsedTime;
		fCannonTemperature = max(0.0f, fCannonTemperature - RemainingPowerUp - fElapsedTime);		//cool down
		//Generate a bullet as mouse button is held down
		if (GetMouse(0).bHeld)
		{
			if (fFireRateAcc >= fFireRate && fCannonTemperature < 5.0f)
			{
				//srand(time(NULL));
				//fFireRateAcc -= fFireRate;
				fFireRateAcc = 0;
				fCannonTemperature += 0.6f;		//heat up
				//Generate one Bullet for Player
				sBullet b;
				olc::SOUND::PlaySample(sndFireCannon);
				float d = 1.0f / (sqrtf(dx * dx + dy * dy));
				if (nLevel > 30 && nLevel < 50) { //FIRE TRIPLE CANNONS!
					b.px = Player.px + 40 * cos(fHeading);
					b.py = Player.py + 40 * sin(fHeading);
					b.vx = dx * d * 400.0f;
					b.vy = dy * d * 400.0f;
					b.hitPoint = 20;
					b.speed = 10 + FireSpeed;
					b.speedFactor = 0.66f;
					b.hasExploded = false;
					b.bEnemyFire = false;
					listBullets.push_back(b);
					b.hitPoint = 10;
					b.vx = dx * d * 400.0f + 20;
					b.vy = dy * d * 400.0f;
					listBullets.push_back(b);
					b.hitPoint = 10; 
					b.vx = dx * d * 400.0f;
					b.vy = dy * d * 400.0f + 20;
					listBullets.push_back(b);
					
					// b.vx = dx * 1.1f * d * 400.0f;
					// b.vy = dy * 1.1f * d * 400.0f;
					//
				}
				else if (nLevel >= 50 ) { //FIRE 5 CANNONS!
					b.px = Player.px + 40 * cos(fHeading);
					b.py = Player.py + 40 * sin(fHeading);
					b.vx = dx * d * 400.0f;
					b.vy = dy * d * 400.0f;
					b.hitPoint = 30;
					b.speed = 10 + FireSpeed;
					b.speedFactor = 0.66f;
					b.hasExploded = false;
					b.bEnemyFire = false;
					listBullets.push_back(b);
					b.hitPoint = 20;
					b.vx = dx * d * 400.0f + 15;
					b.vy = dy * d * 400.0f;
					listBullets.push_back(b);
					b.hitPoint = 20;
					b.vx = dx * d * 400.0f;
					b.vy = dy * d * 400.0f + 15;
					listBullets.push_back(b);
					b.hitPoint = 20;
					b.vx = dx * d * 400.0f - 40;
					b.vy = dy * d * 400.0f;
					listBullets.push_back(b);
					b.hitPoint = 20;
					b.vx = dx * d * 400.0f;
					b.vy = dy * d * 400.0f - 40;
					listBullets.push_back(b);
				}
				else {
					b.px = Player.px + 40 * cos(fHeading);
					b.py = Player.py + 40 * sin(fHeading);
					b.vx = dx * d * 400.0f;
					b.vy = dy * d * 400.0f;
					b.hitPoint = 20;
					b.speed = 10 + FireSpeed;
					b.speedFactor = 0.66f;
					b.hasExploded = false;
					b.bEnemyFire = false;
					listBullets.push_back(b);
				}
					

			}
		}
		
		if (GetMouse(1).bHeld) { //right click
			updateWP(targetWPid, float(nMouseX), float(nMouseY)); //update target WP
			//todo: add some sound and visual FX to inform the Player!
		}
		
		//update bullets
		for (auto &b : listBullets)
		{
			b.px += b.vx * fElapsedTime;
			b.py += b.vy * fElapsedTime;
			for (auto &o : listEnemies)
			{
				if (!b.bEnemyFire && (distance(o.px, o.py, b.px, b.py) < 1600))	//check enemies vs playerbullets
				{
					o.health = int(o.health - b.hitPoint);
					if (o.health <= 0) {	//enemy is killed
						gameScore += int(1000.0f + o.firePower * 10.0f);	// was 1000
					}
					o.fireRate = o.fireRate * 1.2f;
					o.px -= o.vx*10.0f*fElapsedTime;	//go back a bit
					o.py -= o.vy*10.0f*fElapsedTime;
					o.vx = o.vx * b.speedFactor;
					o.vy = o.vy * b.speedFactor;
					
					b.hasExploded = true;
					olc::SOUND::PlaySample(sndExplode);
					sExplosion x;
					x.px = o.px;
					x.py = o.py;
					x.sizeMax = b.hitPoint;
					x.currentSize = 0;
					listExplosions.push_back(x);

				}
			}
			if ((abs(Player.px - b.px) < 30) && (abs(Player.py - b.py) < 30))	//check bullets vs Player
			{
				Player.health -= b.hitPoint;
				Player.vx = Player.vx * b.speedFactor;
				Player.vy = Player.vy * b.speedFactor;
				b.hasExploded = true;
				sExplosion x;
				x.px = Player.px;
				x.py = Player.py;
				x.sizeMax = b.hitPoint;
				x.currentSize = 0;
				listExplosions.push_back(x);
				if (Player.health <= 0) {
					//GAME OVER

				}
			}
		}
		listBullets.remove_if([&](const sBullet &b) {return (b.px < 0) || (b.py < 0) || (b.px > ScreenWidth()) || (b.py > ScreenHeight()) || b.hasExploded; });

		//update explosions
		for (auto &x : listExplosions)
		{
			x.currentSize += 2.5;
			for (auto &e : listEnemies)
			{
				float temp_dist = distance(e.px, e.py, x.px, x.py);
				// add collateral damage code here
				if ((temp_dist >1000.0f) & ( temp_dist < 10000.0f)) {
					int CollateralDamage = std::rand() % 3;
					e.health = e.health - 10 * CollateralDamage;		// collateral damage
					if (e.health <= 0) {	//enemy is killed
						//gameScore += e.firePower * 10;	// was 1000
						gameScore += int(1000.0f + e.firePower * 10.0f);	// was 1000
					}
					e.fireRate = e.fireRate * 1.2f;
					e.px -= e.vx*10.0f*fElapsedTime;	//go back a bit
					e.py -= e.vy*10.0f*fElapsedTime;
					e.vx = e.vx * 0.95f;		//slow it down a bit
					e.vy = e.vy * 0.95f;

					olc::SOUND::PlaySample(sndExplode);

				}
			}
		}
		listExplosions.remove_if([&](const sExplosion &x) {return (x.px < 0) || (x.py < 0) || (x.px > ScreenWidth()) || (x.py > ScreenHeight()) || (x.currentSize > x.sizeMax); });

		//update enemies
		for (auto &e : listEnemies)
		{
			e.px += e.vx * fElapsedTime;
			e.py += e.vy * fElapsedTime;
			//float hdgReq = atan2((Player.py - e.py), (Player.px - e.px)) + 1.5708f;
			//if (hdgReq < 0) hdgReq += 2.0f * PI;
			if (distance(Player.px, Player.py, e.px, e.py)< 1200) {
				Player.health -= 600;	//player receives damage 
				e.health = -1; // enemy is dead upon collision
				olc::SOUND::PlaySample(sndExplode);
				sExplosion x;
				x.px = Player.px;
				x.py = Player.py;
				x.sizeMax = 10;
				x.currentSize = 0;
				listExplosions.push_back(x);
			}
			e.hdg = atan2((Player.py - e.py), (Player.px - e.px)) + 1.5708f;		//todo: implement quickest turn
			//e.hdg += 1.5708f;
			e.vx = (0.5f * (Player.px - 20.0f * cos(e.hdg - 1.5708f) - e.px));		//stand at an offset
			e.vy = (0.5f * (Player.py - 20.0f * sin(e.hdg - 1.5708f) - e.py));		//stand at an offset
			e.vx = Saturate(e.vx, -e.speedMax, +e.speedMax);
			e.vy = Saturate(e.vy, -e.speedMax, +e.speedMax);
			e.fireRateAcc += fElapsedTime;
			if (e.fireRateAcc > e.fireRate) {
				//generate a bullet from the Enemy, directed at its heading with some randomness
				sBullet b;
				float rndOffset = (std::rand() % 100) / 1000.0f - 0.05f;  // (-0.5, +0.5)
				b.px = e.px + 80 * cos(e.hdg + rndOffset - 1.5708f);
				b.py = e.py + 80 * sin(e.hdg + rndOffset - 1.5708f);
				b.hitPoint = e.firePower;
				b.vx = 20 * e.firePower * cos(e.hdg + rndOffset - 1.5708f);
				b.vy = 20 * e.firePower * sin(e.hdg + rndOffset - 1.5708f);
				b.hasExploded = false;
				b.bEnemyFire = true;
				b.speed = e.fireSpeed;
				b.speedFactor = 0.95f;
				listBullets.push_back(b);
				e.fireRateAcc = 0.0f;	//reload
			}
			//check each *other* enemy for collision
			//for (auto &o : listEnemies)
			//{
			//	if ((abs(o.px - e.px) < 40) && (abs(o.py - e.py) < 40) && (o.px != e.px) && (o.py != e.py))	//check collision
			//	{
			//		o.health = o.health - 1;
			//		/*o.vx = o.vx * 0.8;
			//		o.vy = o.vy * 0.8;*/
			//		e.health = e.health - 1;
			//		/*e.vx = e.vx * 0.8;
			//		e.vy = e.vy * 0.8;*/
			//	}
			//}
		}
		listEnemies.remove_if([&](const sEnemy &e) {return (e.px < 0) || (e.py < 0) || (e.px > ScreenWidth()) || (e.py > ScreenHeight()) || e.health < 0; });

		//update Powerups
		for (auto &pu : listPowerups)
		{
			pu.px += pu.vx * fElapsedTime;
			pu.py += pu.vy * fElapsedTime;
			pu.timer += fElapsedTime;
			if ((fabs(pu.px - Player.px) < 30) && (fabs(pu.py - Player.py) < 30)) {
				Player.health += pu.fPlayerHealth;
				maxSpeed = pu.fPlayerMaxSpeed;
				pu.timer = pu.timeout;
				fFireRate = pu.fPlayerFireRate * fFireRate;
				FireSpeed = 10;
				gameScore += int(pu.fScore);
				sActivePowerup apu;
				apu.timer = 0.0f;
				apu.timeout = 10.0f + fTimePowerUp;
				listActivePowerups.push_back(apu);
				olc::SOUND::PlaySample(sndPowerUp1);
			}
		}
		listPowerups.remove_if([&](const sPowerup &pu) {return (pu.px < 0) || (pu.py < 0) || (pu.px > ScreenWidth()) || (pu.py > ScreenHeight()) || pu.timer >= pu.timeout; });

		//update ActivePowerups
		for (auto &apu : listActivePowerups)
		{
			apu.timer += fElapsedTime;
			if (apu.timer >= apu.timeout) {
				//todo: stop the effects of this powerup
				fFireRate = fFireRate / 0.85f;
			}
			else {
			}
		}
		listActivePowerups.remove_if([&](const sActivePowerup &apu) {return apu.timer >= apu.timeout; });

SetDrawTarget(buffBack);
//fWorldX = 0.0f;		//200 * cos(fGlobalTime * 2);
//fWorldY = 0.0f;		//400 * sin(fGlobalTime * 2);
DrawPartialSprite(0, 0, sprBackground, int(300 + fWorldX), int(200 + fWorldY), ScreenWidth(), ScreenHeight());	//move the background if the player approaches the sides (until size of image of course!)
fGlobalTime += fElapsedTime;

//draw Patrol Path
{
	for (auto &WP : listPath)
	{
		FillCircle(int(WP.x - fWorldX), int(WP.y -fWorldY), 6, olc::CYAN);
		FillRect(int(WP.x - fWorldX), int(WP.y - fWorldY), int(3 * WP.id), 2, olc::BLUE);
	}
}
//draw TANK // draw Player
{
	// adjust world offset (camera view) wrt player
	if ((Player.px - fWorldX) < 100) fWorldX -= 50 * fElapsedTime;
	else if ((Player.px - fWorldX) > ScreenWidth() - 100) fWorldX +=  50 * fElapsedTime;
	if ((Player.py - fWorldY) < 100) fWorldY -=  50 * fElapsedTime;
	else if ((Player.py - fWorldY) > ScreenHeight() - 100) fWorldY +=  50 * fElapsedTime;
	olc::GFX2D::Transform2D t;
	//t.Translate(-70.0f, -120.0f);
	t.Translate(-100, -50); //dene yan�l.
	t.Scale(0.4f, 0.4f);
	t.Rotate(fBearing);
	t.Translate(Player.px - fWorldX, Player.py - fWorldY);		// -fWorldX, -fWorldY
	SetPixelMode(olc::Pixel::MASK);
	olc::GFX2D::DrawSprite(sprPlayer, t);
	//SetPixelMode(olc::Pixel::NORMAL);


	//Draw Turret
	olc::GFX2D::Transform2D tTurret;
	tTurret.Translate(-85 + tunerX, -50 + tunerY);
	// cout << "x:" << tunerX << " | y: " << tunerY << endl;
	tTurret.Scale(0.4f, 0.4f);		// was 0.4 todo: maybe animate when firing bullets?
	tTurret.Rotate(fHeading);
	tTurret.Translate(Player.px - fWorldX, Player.py - fWorldY);		// -fWorldX, -fWorldY
	/*SetPixelMode(olc::Pixel::MASK);*/
	olc::GFX2D::DrawSprite(sprTurret, tTurret);
	SetPixelMode(olc::Pixel::NORMAL);
}
//draw enemies
{
	SetPixelMode(olc::Pixel::MASK);
	for (auto &e : listEnemies)
	{
		olc::GFX2D::Transform2D tE;
		tE.Translate(-70.0f, -120.0f);
		tE.Rotate(e.hdg);
		tE.Scale(0.30f, 0.30f);		//was 0.4
		tE.Translate(e.px - fWorldX, e.py - fWorldY);

		olc::GFX2D::DrawSprite(e.sprEnemy, tE);
		DrawRect(int(e.px - fWorldX - 20.0f), int(e.py - fWorldY - 20.0f), 40, 5, olc::BLACK);
		FillRect(int(e.px - fWorldX - 20.0f), int(e.py - fWorldY - 20.0f), int(e.health / 4 - 1.0f), 4, olc::GREY);

	}
	SetPixelMode(olc::Pixel::NORMAL);
}
//draw Powerups
{
	SetPixelMode(olc::Pixel::MASK);
	for (auto &pu : listPowerups)
	{
		olc::GFX2D::Transform2D tE;
		tE.Translate(-120.0f, -140.0f);
		//tE.Rotate(e.hdg);
		tE.Scale(0.2f, 0.2f);
		tE.Translate(pu.px - fWorldX, pu.py - fWorldY);
		olc::GFX2D::DrawSprite(pu.sprPowerUp, tE);
	}
	SetPixelMode(olc::Pixel::NORMAL);
}
//draw ActivePowerUps
{
	RemainingPowerUp = 0;
	for (auto &apu : listActivePowerups)
	{
		RemainingPowerUp = max(apu.timeout - apu.timer, RemainingPowerUp);
	}

	if (RemainingPowerUp > 0) {
		SetPixelMode(olc::Pixel::MASK);
		FillRect(20, 55, int(RemainingPowerUp * 4.0f), 16, olc::BLUE);
		//DrawString(20, 60, "PU", olc::WHITE);
		SetPixelMode(olc::Pixel::NORMAL);
		fTimePowerUp = RemainingPowerUp;
		//DrawString(20, 60, "Dry Ice", olc::WHITE);	//debug
	}
}
//draw bullets
{
	for (auto &b : listBullets)
	{
		FillCircle(int(b.px - fWorldX), int(b.py + 6 - fWorldY), int(b.hitPoint / 4.0f - 1), olc::BLACK);		//cast some shadow
		FillCircle(int(b.px - fWorldX), int(b.py - fWorldY), int(b.hitPoint / 4.0f), olc::DARK_RED);
	}
}
//draw Explosions
{
	for (auto &x : listExplosions) {
		FillCircle(int(x.px - fWorldX), int(x.py - fWorldY), int(x.currentSize), olc::DARK_RED);
		DrawCircle(int(x.px - fWorldX), int(x.py - fWorldY), int(x.currentSize), olc::RED);
	}
}
// Draw cross hair
{
	olc::Pixel crosshaircolor = olc::WHITE;
	if (float(distance(float(nMouseX), float(nMouseY), Player.px, Player.py)) < 40000.0f) {
		crosshaircolor = olc::WHITE;
		DrawCircle(nMouseX, nMouseY, 7, crosshaircolor);
	}
	else {
		crosshaircolor = olc::DARK_GREEN;
	}
	
	DrawLine(nMouseX, nMouseY - 10, nMouseX, nMouseY + 10, crosshaircolor);
	DrawLine(nMouseX - 10, nMouseY, nMouseX + 10, nMouseY, crosshaircolor);
		}
		//Draw Game Score and Player Health
		{
			int BestScore;
			if (gameScore < HiScore){
				BestScore = HiScore;
			}
			else {
				BestScore = gameScore;
			}
			DrawString(21, 11, "P1", olc::BLACK, 2);
			DrawString(20, 10, "P1", olc::WHITE, 2);
			DrawString(121, 11, to_string(gameScore), olc::BLACK, 2);
			DrawString(120, 10, to_string(gameScore), olc::WHITE, 2);
			//cout << BestPlayerName << " ";
			DrawString(21, 31, BestPlayerName[0] + BestPlayerName[1] + BestPlayerName[2], olc::BLACK, 2);
			DrawString(20, 30, BestPlayerName[0] + BestPlayerName[1] + BestPlayerName[2], olc::RED, 2);
			DrawString(121, 31, to_string(BestScore), olc::BLACK, 2);
			DrawString(120, 30, to_string(BestScore), olc::RED, 2);
			

			DrawString(ScreenWidth() - 160, 11, "LEVEL:", olc::DARK_BLUE, 2);
			DrawString(ScreenWidth() - 159, 10, "LEVEL:", olc::BLUE, 2);
			DrawString(ScreenWidth() - 60, 11, to_string(nLevel), olc::DARK_CYAN, 2);
			DrawString(ScreenWidth() - 59, 10, to_string(nLevel), olc::CYAN, 2);
			int HealthBarLength = 100;
			float dHealth = 1000.0f / HealthBarLength;
			DrawRect(ScreenWidth() / 2 - HealthBarLength / 2, HealthBarLength / 5, HealthBarLength, HealthBarLength / 5, olc::BLACK);
			
			if (Player.health < 200) FillRect(ScreenWidth() / 2 - HealthBarLength / 2 + 1, 21, int(Player.health / dHealth) - 1, 19, olc::RED);
			else FillRect(ScreenWidth() / 2 - HealthBarLength / 2 + 1, 21, int(Player.health / dHealth) - 1, 19, olc::GREEN);
			
			DrawRect(int(Player.px - fWorldX - HealthBarLength / 8.0f), int(Player.py - fWorldY - 40.0f), int(HealthBarLength / 4.0f), int(HealthBarLength / 20.0f), olc::BLACK);
			FillRect(int(Player.px - fWorldX - HealthBarLength / 8.0f), int(Player.py - fWorldY - 40.0f), int(Player.health / dHealth /4.0f - 1.0f), int(HealthBarLength / 20.0f), olc::GREEN);

			FillRect(20, 80, int(fCannonTemperature*4.0f), 16, (fCannonTemperature<4)? olc::BLUE : (fCannonTemperature<5) ? olc::DARK_RED : olc::RED );

			//Draw to Screen now
			SetDrawTarget(nullptr);
			DrawSprite(0, 0, buffBack);
		}

		//Draw Autopilot Stuff
		{
			//char tmpText[20];
			//todo: mousewheel can be accumulated to some variable, dunno what, yet. :)
			//sprintf_s(tmpText, "MouseWheel: %d", nMouseWheel);
			//DrawString(ScreenWidth() / 2 - 25, 60, tmpText, olc::WHITE);
			DrawString(ScreenWidth()/2 - 25, 10, "AP:", olc::WHITE, 1);
			if (bAutoNav) {
				FillCircle(ScreenWidth() / 2 + 5, 14, 5, olc::CYAN);
			}
			FillRect(ScreenWidth() / 2 + 20, 15, int(tunerX * 5), int(tunerY * 5), olc::GREY);
		}
	}
	void GameOver(float fElapsedTime) {
		if (bStateEntry) {
			//
			// cout << "entered GameOver!";
			bStateEntry = false;
		}
		SetDrawTarget(buffBack);
		FillRect(0, 0, ScreenWidth(), ScreenHeight(), olc::DARK_RED);
		olc::GFX2D::Transform2D t;
		t.Scale(1.8f, 1.8f);
		t.Translate(-160.0f, 0.0f);
		olc::GFX2D::DrawSprite(sprSplashScreen, t);
		//DrawSprite(0, 0, sprSplashScreen);
		DrawString(ScreenWidth() / 2 - 200, ScreenHeight() / 2 - 200, "GAME OVER!", olc::BLACK, 5);
		DrawString(ScreenWidth() / 2 - 200 - 4, ScreenHeight() / 2 - 204, "GAME OVER!", olc::RED, 5);
		char scoreText[20];
		//sprintf_s(scoreText, "SCORE: %d", gameScore);
		snprintf(scoreText, 20, "SCORE: %d", gameScore); // Linux compatibility, seems to work :) 4.4.2021
		DrawString(ScreenWidth() / 2 - 150, ScreenHeight() / 2 , scoreText, olc::BLACK, 3);
		DrawString(ScreenWidth() / 2 - 150 - 4, ScreenHeight() / 2 - 4, scoreText, olc::WHITE, 3);

		if (gameScore > HiScore) {
			//todo: write to file (in sorted order?)
			std::fstream fScores("hiscores.txt", std::ios_base::out);
			fScores << gameScore << " " << "aaa" ;
			fScores.close();

			float animate = 15*(sin(3*fGlobalTime)+cos(2*fGlobalTime));
			fGlobalTime += fElapsedTime;
			DrawString(int(ScreenWidth() / 2.0f) - 150, int(ScreenHeight() / 2.0f + 75.0f + animate), "NEW HI SCORE!!!", olc::BLACK, 3);
			DrawString(int(ScreenWidth() / 2.0f) - 154, int(ScreenHeight() / 2.0f + 71.0f + animate), "NEW HI SCORE!!!", olc::CYAN, 3);

		}
		//wait for space key to start game
		
		DrawString(int(ScreenWidth() / 2.0f) - 250, int(ScreenHeight() / 2.0f) + 150, "PRESS SPACE TO START AGAIN.", olc::BLACK, 2);
		DrawString(int(ScreenWidth() / 2.0f) - 254, int(ScreenHeight() / 2.0f) + 146, "PRESS SPACE TO START AGAIN.", olc::WHITE, 2);
		
		DrawString(int(ScreenWidth() / 2.0f) - 250, int(ScreenHeight() * 2.0f / 3.0f) + 150, "PRESS ESC TO QUIT.", olc::BLACK, 2);
		DrawString(int(ScreenWidth() / 2.0f) - 254, int(ScreenHeight() * 2.0f / 3.0f) + 146, "PRESS ESC TO QUIT.", olc::WHITE, 2);

		SetDrawTarget(nullptr);
		DrawSprite(0, 0, buffBack);
	}
public:
	float Saturate(float val, float limitmin, float limitmax) {
		return (max(min(val, limitmax), limitmin));
	}

	bool OnUserCreate() override
	{
		// Called once at the start, so create things here
		// todo: report ALL missing or bad files (if sndXXX == -1)
		olc::SOUND::InitialiseAudio();
		sndSplashScreen = olc::SOUND::LoadAudioSample("Resources/music/SplashScreen.wav");
		sprSplashScreen = new olc::Sprite("./Resources/altay-01.jpg");

		nGameState = 0;
		prevGameState = -1;
		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{

		if (nGameState == 0) {	//SplashScreen
			if (prevGameState != nGameState) { //runs once upon entry into SplashScreen state
				olc::SOUND::StopAll();
				olc::SOUND::PlaySample(sndSplashScreen, true); // Starts to Play the sample in looping mode
				//ReadGameStoryFromFile();
				sStory = readStory("GameStory.txt");
				nDisplayTextPlace = 0;
				nDisplayTextStart = 0;
				//InitializeGame(); //set all global values to initial values for a fresh game
				// Read HiScores from file
				{
					std::fstream fScores("hiscores.txt", std::ios_base::in);
					struct HiScoreData {
						int score;
						char name[3];
					};
					HiScoreData hiscores[10];
					int numscores = 0;
					while (fScores >> hiscores[numscores].score >> hiscores[numscores].name) {
						numscores++;
					}
					fScores.close();
					int BestScore = -1;
					for (int i = 0; i < numscores; i++)
					{
						if (hiscores[i].score > BestScore) {
							BestScore = hiscores[i].score;
							BestPlayerName[0] = hiscores[i].name[0];
							BestPlayerName[1] = hiscores[i].name[1];
							BestPlayerName[2] = hiscores[i].name[2];
						}
					}

					HiScore = BestScore;
				}
				// set Initial Values
				//Clear(olc::BLACK);


				// SET Waypoints for the auto-patrol PATH

				float WPX[4];
				float WPY[4];
				WPX[0] = 100.0f;					WPY[0] = 100.0f;
				WPX[1] = 100.0f;					WPY[1] = ScreenHeight() - 100.0f;
				WPX[2] = ScreenWidth() - 100.0f;	WPY[2] = ScreenHeight() - 100.0f;
				WPX[3] = ScreenWidth() - 100.0f;	WPY[3] = 100.0f;
				listPath.clear();
				for (int i = 0; i < 4; i++) {
					tempWP.id = i + 1;
					tempWP.x = WPX[i];
					tempWP.y = WPY[i];
					listPath.push_back(tempWP);
				}
				// waypoints are stored in list listPath.

				Player.px = ScreenWidth() / 2.0f;
				Player.py = ScreenHeight() / 2.0f;
				Player.vx = 0.0f;
				Player.vy = 0.0f;
				Player.health = 1000.0f;
				fFireRate = 0.25f;
				fFireRateAcc = 0.0f;
				FireSpeed = 0.0f;		//Fire speed for Player
				maxSpeed = 2.0f;		//max speed in U and V for Player
				listBullets.remove_if([&](const sBullet &b) {return true; });
				listEnemies.remove_if([&](const sEnemy &e) {return true; });
				listExplosions.remove_if([&](const sExplosion &e) {return true; });
				listPowerups.remove_if([&](const sPowerup &pu) {return true; });
				RemainingPowerUp = 0;
				nLevel = 1;
				gameScore = 0;
				fGlobalTime = 0.0f;
			}
			SplashScreen(fElapsedTime);
			prevGameState = nGameState;
			if (GetKey(olc::Key::SPACE).bPressed) nGameState = 1;
		}
		else if (nGameState == 999) {				//GAME OVER
			if (prevGameState != nGameState) {
				olc::SOUND::StopAll();
				olc::SOUND::PlaySample(sndGameOverOnce);   // Plays the gameover effect
				olc::SOUND::PlaySample(sndGameOver, true); // Plays the sample in looping mode
			}
			GameOver(fElapsedTime);
			prevGameState = nGameState;
			if (GetKey(olc::Key::SPACE).bPressed) nGameState = 0;
			if (GetKey(olc::Key::ESCAPE).bPressed) nGameState = -1;
		}
		else if (nGameState == 1) {				// GAME IS ON
			if (prevGameState != nGameState) { // ENTRY
				olc::SOUND::StopSample(sndSplashScreen);
				olc::SOUND::StopSample(sndGameOver);
				// TODO: READ ALL ASSETS HERE, while writing "LOADING GAME"
				// TODO : PLAY SOME SHORT SOUND
				if (!AssetsLoaded) {
					cout << "\n Loading sounds...\n";
					sndGameBackground = olc::SOUND::LoadAudioSample("Resources/music/GameBackground.wav");
					sndGameOver = olc::SOUND::LoadAudioSample("Resources/music/SplashScreen.wav");	//to be replaced with GameOver again?
					sndGameOverOnce = olc::SOUND::LoadAudioSample("Resources/soundFX/GameOverOnce.wav");
					sndFireCannon = olc::SOUND::LoadAudioSample("Resources/soundFX/FireCannon.wav");
					sndExplode = olc::SOUND::LoadAudioSample("Resources/soundFX/Explode.wav");
					sndPowerUp1 = olc::SOUND::LoadAudioSample("Resources/soundFX/PowerUp1.wav");		//test
					if (sndPowerUp1 == -1) sndPowerUp1 = sndExplode; //load default sound if PowerUp1.wav is broken!
					sndAutopilotOn = olc::SOUND::LoadAudioSample("Resources/soundFX/autopilot.wav");
					sndAutopilotOff = olc::SOUND::LoadAudioSample("Resources/soundFX/coin.wav");

					cout << "\n Loading sprites...\n";
					sprPlayer = new olc::Sprite("Resources/tank.png");
					sprTurret = new olc::Sprite("Resources/tank_turret.png");
					sprBackground = new olc::Sprite("Resources/Planet.png");
					sprEnemy1 = new olc::Sprite("Resources/enemy.png");
					sprEnemy2 = new olc::Sprite("Resources/enemy2.png");
					buffBack = new olc::Sprite(ScreenWidth(), ScreenHeight());
					sprPowerUp_01 = new olc::Sprite("Resources/powerups/PowerUp_01.png");
					cout << "LOADING COMPLETE!\n";
					AssetsLoaded = true;
				}
				olc::SOUND::PlaySample(sndGameBackground, true);
				fGlobalTime = 0.0f;
			}
			if (!bGamePaused) PlayGame(fElapsedTime);
			if (bGamePaused) {
				fTimerPause += fElapsedTime;
				FillCircle(ScreenWidth() / 2, ScreenHeight() / 2, ScreenHeight() / 3, olc::RED);
				DrawString(ScreenWidth() / 2 - 150, ScreenHeight() / 2 - 100, "Game Paused... \n PRESS SPACE TO PLAY ! \n PRESS Q to Quit...", olc::WHITE, 2);
				DrawString(ScreenWidth() / 2 - 150, ScreenHeight() / 2 + 20, "PRESS TAB to TOGGLE AUTOPILOT\n 1-2-3-4: GO TO WAYPOINT # \n E and R keys: Adjust Autopilot x-Speed\n T and Y keys: Adjust Autopilot y-Speed... ", olc::WHITE, 1);

				if (fTimerPause > 1.0 && GetKey(olc::Key::SPACE).bHeld) {
					bGamePaused = false;
					fTimerPause = 100.0f;
				}
				if (fTimerPause > 1.0 && GetKey(olc::Key::Q).bHeld) {
					nGameState = 999;
				}
			}
			prevGameState = nGameState;
			if (Player.health < 0) nGameState = 999;
			// if (GetKey(olc::Key::SPACE).bPressed) nGameState = 999; //FOR TEST
		}
		else if (nGameState == -1) {				//QUIT NOW
			return false;
		}
		return true;
	}

	bool OnUserDestroy()
	{
		olc::SOUND::DestroyAudio();
		return true;
	}
};


int main()
{
	
	bool bFullScreen = false;
	cout << "PRESS ENTER TO PLAY FULL SCREEN! Or Press any key + Enter to play in Window Mode.";
	cout << endl;
	while (true) {
		//cout << getchar();
		int temp = getchar();
		if (temp == 10) {
			bFullScreen = true;
			break;
		}
		else if (temp != 10) {
			bFullScreen = false;
			break;
		}
	}

	Altay game;
	if (game.Construct(1000, 800, 1, 1, bFullScreen)) //800 x 600
		game.Start();

	return 0;
}
 
