#include "pch.h"
#include "olcPixelGameEngine.h"
#include "olcPGEX_Graphics2D.h"

using namespace std;
const double PI = 3.141592653589793238463;

/* 
BUGS:
-- Game continues after Game Over.
-- sometimes several enemies spawn simultaneously (there must be a limit to size_of_enemy_list)

TODO:
-- add sounds
-- info popup for powerups
-- add a super bomb powerup

++ powerup effectivity timers			DONE! 23.12.2018
++ Init Game memory (kill and reinit)	DONE! 23.12.2018
++ Game reinits after Game Over screen.	DONE! 23.12.2018
++ Levels added!						DONE! 12.12.2018
++ Balance gameplay						DONE! 9.12.2018
++ display HealthBar					DONE! 9.12.2018
++ Player PowerUps						DONE! 8.12.2018
++ added GameOver						DONE! 8.12.2018
++ just noticed on Javidx9's livestream that the enemies are not firing bullets. They are firing now.
++ cancel The Text-Size animation		DONE
++ enemies run towards the player		DONE! 2.12.2018
++ enemy spawn timing is improved		DONE! 2.12.2018
++ enemies do not damage each other		DONE! 2.12.2018
++ Player HealthLevel					DONE! 1.12.2018
++ Enemy HealthLevel					DONE! 1.12.2018
++ Enemy fires back						DONE! 1.12.2018
++ bullets explode near an enemy		DONE! 30.11.2018
++ multiple enemies						DONE! 28.11.2018
++ Different Enemies					DONE! 28.11.2018

*/


class Altay : public olc::PixelGameEngine
{
public:
	Altay()
	{
		sAppName = "Tank Mission ALTAY, 0.7a";
	}

private:
	float fGlobalTime = 0.0f;
	float fWorldX = 0.0f;
	float fWorldY = 0.0f;
	int gameScore;	
	int nGameState;		//splash screen,  game, endGame, todo: menu
	int nLevel = 1;

	olc::Sprite *buffBack;
	olc::Sprite *sprBackground;				
	olc::Sprite *sprSplashScreen;
	olc::Sprite *sprPowerUp_01;

	struct sPlayer {
		float px = 0.0f;
		float py = 0.0f;
		float vx = 0.0f; // speed in X direction
		float vy = 0.0f;	// speed in Y direction
		float health = 1000.0f;
		float speed = 0.80f;
	};// Define player variables
	sPlayer Player;
	float fFireRate = 0.25f;
	float fFireRateAcc = 0.0f;
	float FireSpeed = 0.0f;
	float maxSpeed = 2.0f;		//max speed in U and V
	olc::Sprite *sprPlayer;
	
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

	void SplashScreen() {
		//Clear(olc::BLACK);
		Player.px = ScreenWidth() / 2;
		Player.py = ScreenHeight() / 2;
		Player.vx = 0;
		Player.vy = 0;
		Player.health = 1000.0f;
		fFireRate = 0.25f;
		fFireRateAcc = 0.0f;
		FireSpeed = 0.0f;		//Fire speed for Player
		maxSpeed = 2.0f;		//max speed in U and V for Player
		std::srand(std::time(nullptr));
		SetDrawTarget(buffBack);
		DrawSprite(-300, -200, sprSplashScreen);
		//wait for space key to start game
		DrawString(ScreenWidth() / 2 - 250, ScreenHeight() / 2, "PRESS SPACE TO START...", olc::BLACK, 3);
		DrawString(ScreenWidth() / 2 - 250 - 4, ScreenHeight() / 2 - 4, "PRESS SPACE TO START...", olc::WHITE, 3);
		//Draw to Screen now
		SetDrawTarget(nullptr);
		DrawSprite(0, 0, buffBack);
		listBullets.remove_if([&](const sBullet &b) {return true; });
		listEnemies.remove_if([&](const sEnemy &e) {return true; });
		listExplosions.remove_if([&](const sExplosion &e) {return true; });
		listPowerups.remove_if([&](const sPowerup &pu) {return true; });
		nLevel = 1;
		gameScore = 0;
		fGlobalTime = 0;
		//todo: run function "INIT();"
	}
	void PlayGame(float fElapsedTime) {
		int nMouseX = GetMouseX();
		int nMouseY = GetMouseY();
		//update Player
		float dx = (float)nMouseX - Player.px;
		float dy = (float)nMouseY - Player.py;
		float fHeading = atan2(dy, dx) + 1.5708f;	//rotate 90 deg to match with image
		// Move Player
		if (GetKey(olc::Key::A).bHeld) Player.vx -= Player.speed * fElapsedTime;
		if (GetKey(olc::Key::D).bHeld) Player.vx += Player.speed * fElapsedTime;
		if (GetKey(olc::Key::W).bHeld) Player.vy -= Player.speed * fElapsedTime;
		if (GetKey(olc::Key::S).bHeld) Player.vy += Player.speed * fElapsedTime;
		if (GetKey(olc::Key::E).bHeld) {
			Player.vx = 0.0;
			Player.vy = 0.0;
			Player.px = ScreenWidth() / 2;
			Player.py = ScreenHeight() / 2;
		}
		Player.vx = 0.99 * Player.vx;
		Player.vy = 0.99 * Player.vy;

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
		if (gameScore > nLevel * 10000) nLevel++;

		//if (GetKey(olc::Key::SPACE).bPressed) Player.health = -1 * Player.health;		// for TEST

		if (listEnemies.size() < nLevel) {
			//Generate one Enemy (approx every 2 seconds (1/100 chance)
			int dicer = 0;

			dicer = std::rand() % (2500 - int(Player.health)) / nLevel;	//change difficulty according to player Health and LEVEL!

			if (dicer == 0 && nLevel > 3) {
				sEnemy e;
				e.px = std::rand() / RAND_MAX * ScreenWidth() / 4 + ScreenWidth() / 4;
				e.py = std::rand() / RAND_MAX * ScreenHeight() / 4 + ScreenHeight() / 4;
				e.sprEnemy = new olc::Sprite("resources\\enemy.png");
				e.vx = 10 * (std::rand() % 20 - 10);
				e.vy = 10 * (std::rand() % 20 - 10);
				e.health = 100;		//full health
				float dxE = (float)e.px - Player.px;	// enemy should face Player
				float dyE = (float)e.py - Player.py;
				float fEnemyHeading = atan2(dyE, dxE) - 1.5708f;	//rotate 90 deg to match with image
				e.hdg = fEnemyHeading + (std::rand() / RAND_MAX - 0.5)*0.3145;
				e.firePower = 2 * nLevel;
				e.fireRate = 1.2;
				e.fireRateAcc = 0.0f;
				e.fireSpeed = 10;
				e.speedMax = 20;
				listEnemies.push_back(e);
			}
			else if (dicer == 1) {
				sEnemy e;
				e.px = std::rand() / (RAND_MAX / ScreenWidth() * 4) + ScreenWidth() / 4;
				e.py = std::rand() / (RAND_MAX / ScreenHeight() * 4) + ScreenHeight() / 4;
				e.sprEnemy = new olc::Sprite("resources\\enemy2.png");
				e.vx = 10 * (std::rand() % 20 - 10);
				e.vy = 10 * (std::rand() % 20 - 10);
				e.health = 255;		//full health
				float dxE = (float)e.px - Player.px;
				float dyE = (float)e.py - Player.py;
				float fEnemyHeading = atan2(dyE, dxE) - 1.5708f;	//rotate 90 deg to match with image
				e.hdg = fEnemyHeading + (std::rand() / RAND_MAX - 0.5)*0.3145;
				e.firePower = 2 * (nLevel - 2);
				e.fireRate = 1.5;
				e.fireRateAcc = 0.0f;
				e.speedMax = 10;
				listEnemies.push_back(e);
			}
			else if (dicer == 201) {
				sPowerup pu;
				pu.sprPowerUp = new olc::Sprite("resources\\powerups\\PowerUp_01.png");
				pu.px = (std::rand() / (RAND_MAX / ScreenWidth() * 4)) + ScreenWidth() / 2;
				pu.py = (std::rand() / (RAND_MAX / ScreenHeight() * 4)) + ScreenHeight() / 2;
				pu.vx = 0.0f;  //10 * (std::rand() % 20 - 10);
				pu.vy = 0.0f;  //10 * (std::rand() % 20 - 10);
				pu.fBulletHitPoint = 0;
				pu.fBulletSpeedFactor = 0.95f;
				pu.fPlayerFireRate = 0.125f;
				pu.fPlayerHealth = 0.0f;
				pu.fScore = 150;
				pu.fPlayerMaxSpeed = 2.5;
				pu.timeout = 10.0f;
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

		//Generate new bullet as mouse button is held down
		if (GetMouse(0).bHeld)
		{
			if (fFireRateAcc >= fFireRate)
			{
				//srand(time(NULL));
				//fFireRateAcc -= fFireRate;
				fFireRateAcc = 0;

				//Generate one Bullet for Player
				sBullet b;
				float d = 1.0f / (sqrtf(dx * dx + dy * dy));
				b.px = Player.px + 40 * cos(fHeading - 1.5708f);
				b.py = Player.py + 40 * sin(fHeading - 1.5708f);
				b.vx = dx * d * 400.0f;
				b.vy = dy * d * 400.0f;
				b.hitPoint = 20;
				b.speed = 10 + FireSpeed;
				b.speedFactor = 0.95;
				b.hasExploded = false;
				b.bEnemyFire = false;
				listBullets.push_back(b);
			}
		}

		//update bullets
		for (auto &b : listBullets)
		{
			b.px += b.vx * fElapsedTime;
			b.py += b.vy * fElapsedTime;
			for (auto &o : listEnemies)
			{
				if (!b.bEnemyFire && (abs(o.px - b.px) < 40) && (abs(o.py - b.py) < 40))	//check tanks vs bullets
				{
					o.health = o.health - b.hitPoint;
					o.vx = o.vx * b.speedFactor;
					o.vy = o.vy * b.speedFactor;
					b.hasExploded = true;
					sExplosion x;
					x.px = o.px;
					x.py = o.py;
					x.sizeMax = b.hitPoint;
					x.currentSize = 0;
					listExplosions.push_back(x);
					if (o.health <= 0) {
						gameScore += 1000;
					}
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
			x.currentSize += 1;
		}
		listExplosions.remove_if([&](const sExplosion &x) {return (x.px < 0) || (x.py < 0) || (x.px > ScreenWidth()) || (x.py > ScreenHeight()) || (x.currentSize > x.sizeMax); });

		//update enemies
		for (auto &e : listEnemies)
		{
			e.px += e.vx * fElapsedTime;
			e.py += e.vy * fElapsedTime;
			//float hdgReq = atan2((Player.py - e.py), (Player.px - e.px)) + 1.5708f;
			//if (hdgReq < 0) hdgReq += 2.0f * PI;
			e.hdg = atan2((Player.py - e.py), (Player.px - e.px)) + 1.5708f;		//todo: implement quickest turn
			//e.hdg += 1.5708f;
			e.vx = (0.5 * (Player.px - 120 * cos(e.hdg - 1.5708f) - e.px));		//stand at an offset
			e.vy = (0.5 * (Player.py - 120 * sin(e.hdg - 1.5708f) - e.py));		//stand at an offset
			e.vx = Saturate(e.vx, -e.speedMax, +e.speedMax);
			e.vy = Saturate(e.vy, -e.speedMax, +e.speedMax);
			e.fireRateAcc += fElapsedTime;
			if (e.fireRateAcc > e.fireRate) {
				//generate a bullet from the Enemy, directed at its heading.
				sBullet b;
				b.px = e.px + 80 * cos(e.hdg - 1.5708f);
				b.py = e.py + 80 * sin(e.hdg - 1.5708f);
				b.hitPoint = e.firePower;
				b.vx = 20 * e.firePower * cos(e.hdg - 1.5708f);
				b.vy = 20 * e.firePower * sin(e.hdg - 1.5708f);
				b.hasExploded = false;
				b.bEnemyFire = true;
				b.speed = 10;
				b.speedFactor = 0.95;
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
				fFireRate = pu.fPlayerFireRate;
				FireSpeed = 10;
				gameScore += pu.fScore;
				sActivePowerup apu;
				apu.timer = 0.0f;
				apu.timeout = 10.0f;
				listActivePowerups.push_back(apu);
			}
		}
		listPowerups.remove_if([&](const sPowerup &pu) {return (pu.px < 0) || (pu.py < 0) || (pu.px > ScreenWidth()) || (pu.py > ScreenHeight()) || pu.timer >= pu.timeout; });

		//update ActivePowerups
		for (auto &apu : listActivePowerups)
		{
			apu.timer += fElapsedTime;
			if (apu.timer >= apu.timeout) {
				//todo: stop the effects of this powerup
				fFireRate = 0.25f;
			}
		}
		listActivePowerups.remove_if([&](const sActivePowerup &apu) {return apu.timer >= apu.timeout; });

		SetDrawTarget(buffBack);
		//fWorldX = 0.0f;		//200 * cos(fGlobalTime * 2);
		//fWorldY = 0.0f;		//400 * sin(fGlobalTime * 2);
		DrawPartialSprite(0, 0, sprBackground, 300 + fWorldX, 200 + fWorldY, ScreenWidth(), ScreenHeight());	//move the background if the player approaches the sides (until size of image of course!)
		fGlobalTime += fElapsedTime;

		//draw player
		{
			olc::GFX2D::Transform2D t;
			t.Translate(-70.0f, -120.0f);
			t.Scale(0.35, 0.35);		// was 0.4 todo: maybe animate when firing bullets?
			t.Rotate(fHeading);
			t.Translate(Player.px - fWorldX, Player.py - fWorldY);		// -fWorldX, -fWorldY
			SetPixelMode(olc::Pixel::MASK);
			olc::GFX2D::DrawSprite(sprPlayer, t);
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
				tE.Scale(0.30, 0.30);		//was 0.4
				tE.Translate(e.px - fWorldX, e.py - fWorldY);

				olc::GFX2D::DrawSprite(e.sprEnemy, tE);
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
				tE.Scale(0.2, 0.2);
				tE.Translate(pu.px - fWorldX, pu.py - fWorldY);
				olc::GFX2D::DrawSprite(pu.sprPowerUp, tE);
			}
			SetPixelMode(olc::Pixel::NORMAL);
		}
		//draw ActivePowerUps
		{
			SetPixelMode(olc::Pixel::MASK);
			for (auto &apu : listActivePowerups)
			{
				FillRect(20, 55, listActivePowerups.size() * 20, 20, olc::BLUE);
				DrawString(20, 60, "PU", olc::WHITE);
			}
			SetPixelMode(olc::Pixel::NORMAL);
		}
		//draw bullets
		{
			for (auto &b : listBullets)
			{
				FillCircle(b.px - fWorldX, b.py + 6 - fWorldY, b.hitPoint / 4 - 1, olc::BLACK);		//cast some shadow
				FillCircle(b.px - fWorldX, b.py - fWorldY, b.hitPoint / 4, olc::DARK_RED);
			}
		}
		//draw Explosions
		{
			for (auto &x : listExplosions) {
				FillCircle(x.px - fWorldX, x.py - fWorldY, x.currentSize, olc::DARK_RED);
				DrawCircle(x.px - fWorldX, x.py - fWorldY, x.currentSize + 2, olc::RED);
			}
		}
		// Draw cross hair
		{
			DrawCircle(nMouseX, nMouseY, 7, olc::WHITE);
			DrawLine(nMouseX, nMouseY - 10, nMouseX, nMouseY + 10, olc::WHITE);
			DrawLine(nMouseX - 10, nMouseY, nMouseX + 10, nMouseY, olc::WHITE);
		}
		//Draw Game Score and Player Health
		{
			DrawString(21, 11, to_string(gameScore), olc::BLACK, 2);
			DrawString(20, 10, to_string(gameScore), olc::WHITE, 2);
			DrawString(ScreenWidth() - 160, 11, "LEVEL:", olc::DARK_BLUE, 2);
			DrawString(ScreenWidth() - 159, 10, "LEVEL:", olc::BLUE, 2);
			DrawString(ScreenWidth() - 60, 11, to_string(nLevel), olc::DARK_CYAN, 2);
			DrawString(ScreenWidth() - 59, 10, to_string(nLevel), olc::CYAN, 2);
			int HealthBarLength = 100;
			float dHealth = 1000 / HealthBarLength;
			DrawRect(ScreenWidth() / 2 - HealthBarLength / 2, HealthBarLength / 5, HealthBarLength, HealthBarLength / 5, olc::BLACK);
			if (Player.health < 200) FillRect(ScreenWidth() / 2 - HealthBarLength / 2 + 1, 21, int(Player.health / dHealth) - 1, 19, olc::RED);
			else FillRect(ScreenWidth() / 2 - HealthBarLength / 2 + 1, 21, int(Player.health / dHealth) - 1, 19, olc::GREEN);
			//Draw to Screen now
			SetDrawTarget(nullptr);
			DrawSprite(0, 0, buffBack);
		}
	}
	void GameOver() {

		SetDrawTarget(buffBack);
		DrawSprite(-300, -200, sprSplashScreen);
		DrawString(ScreenWidth() / 2 - 150, ScreenHeight() / 2 - 100, "GAME OVER!", olc::BLACK, 4);
		DrawString(ScreenWidth() / 2 - 150 - 4, ScreenHeight() / 2 - 104, "GAME OVER!", olc::RED, 4);
		char scoreText[20];
		sprintf_s(scoreText, "SCORE: %d", gameScore);
		DrawString(ScreenWidth() / 2 - 150, ScreenHeight() / 2 , scoreText, olc::BLACK, 3);
		DrawString(ScreenWidth() / 2 - 150 - 4, ScreenHeight() / 2 - 4, scoreText, olc::WHITE, 3);

		//wait for space key to start game
		
		DrawString(ScreenWidth() / 2 - 250, ScreenHeight() / 2 + 100, "PRESS SPACE TO START AGAIN.", olc::BLACK, 2);
		DrawString(ScreenWidth() / 2 - 250 - 4, ScreenHeight() / 2 + 96, "PRESS SPACE TO START AGAIN.", olc::WHITE, 2);
		//Draw to Screen now
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

		sprPlayer = new olc::Sprite("resources\\tank.png");
		sprBackground = new olc::Sprite("resources\\planet.png");
		sprSplashScreen = new olc::Sprite("resources\\planet.png");
		buffBack = new olc::Sprite(ScreenWidth(), ScreenHeight());

		nGameState = 0;
		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{

		if (nGameState == 0) {
			SplashScreen();
			if (GetKey(olc::Key::SPACE).bPressed) nGameState = 1;
		}
		else if (nGameState == 999) {				//GAME OVER
			GameOver();
			if (GetKey(olc::Key::SPACE).bPressed) nGameState = 0;
		}
		else if (nGameState == 1) {				// GAME IS ON
			PlayGame(fElapsedTime);
			if (Player.health < 0) nGameState = 999;
		}
		return true;
	}
};


int main()
{
	Altay game;
	if (game.Construct(800, 600, 1, 1))
		game.Start();

	return 0;
}
 