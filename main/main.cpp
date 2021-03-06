/*
	*****敬告*****
	你即將開始閱讀為了趕進度所以完全沒優化的code
	如果你有記憶體濫用恐懼症、註解缺乏恐懼症，或模組化低落恐懼症等
	請斟酌閱讀這份code
	如果在閱讀過程有任何不適
	請立即停止閱讀行為
	並找一些療育的貓咪影片之類的來看

	因為閱讀這份code導致的任何身心理創傷
	或是周圍物品的意外損毀
	我方一律不負責相關責任
*/

#define __USE_MINGW_ANSI_STDIO 0
#include <iostream>
#include <cstdlib>
#include <cctype> //用來判斷字元是否為英文或數字
#include <math.h> //簡協運動的三角函數
#include <sstream>
#include <vector>
#include <fstream>
#include "lib/window.h"
#include "lib/timer.h"
#include "lib/gameplay/chartPlayer.h"
#include "lib/global/config.h"

bool replay = false;
void replayBGM()
{
	replay = true;
}

const int FPS=60;
/**
*   簡諧運動
*   @return cos(傳入alurensAlphaTheta後成長一個omega的角度)作為比例，回傳上下限中的該比例對應的值
*   @param &alurensAlphaTheta用來記錄現在在虛擬圓的哪個角度，並會直接以omega遞增/減該值
*   @param low簡諧運動下限
*   @param up簡諧運動上限
*   @param omega虛擬圓角速度
*/
float SHM(float& theta,float low,float up,float omega,bool loop=false)
{
	if((!loop && theta<=3.14) || loop) //如果他不要loop，那theta到pi之後就不要再加了；如果他要loop那就一直放心加吧
		theta+=omega;
	if(theta>6.28 && loop) //他如果要loop才需要在2pi的時候歸零
		theta=0;

    return (up+low)/2+(up-low)/2*cos(theta);
}

int level=1;
bool quit=false,skip=false,finished=false;
int main(int argc,char* args[])
{
	while (!quit)
	{
		ifstream config("data/config",ifstream::in);
		loadConfigurations(config);
		config.close();

		Window::initialize("Eschatology");

		Texture bg("img/bg.jpg");
		Texture* black = new Texture("img/black.png"); //國防布，因為texture根據renderer繪物件，所以若要跨視窗用，就必須用指標

		bg.setDstRect(0,Window::state().h); //讓它填滿螢幕(長填滿，寬依比例縮放)，錨點設為左上
		Texture alurens("img/alurens.png");
		alurens.setDstRect(0,0,alurens.setPoint()); //長寬為原材質長寬，錨點為中心
		alurens.setClipRect(2,2);   //分割成2x2

		Texture title("Eschatology",rgb(255, 255, 255),64);
		Texture title2("pre-demo",rgb(255, 255, 255),32);
		Texture tips("Tap To Start",rgb(255, 255, 255),16);
		tips.setAlpha(0);
		Texture egg("TAB鍵跳過場景",rgb(255, 255, 255),16);
		egg.setAlpha(0);
		Texture egg2("在此按下數字鍵1~3切換 赫里厄斯 難度",rgb(255, 255, 255),16);
		egg2.setAlpha(0);
		title.setDstRect(0,0,title.setPoint()); //長寬為原材質長寬，錨點為中心
		title2.setDstRect(0,0,title2.setPoint());
		tips.setDstRect(0,0,tips.setPoint());

		SDL_Color alurensColor[4]={rgb(255, 128, 128),rgb(255, 255, 128),rgb(211, 128, 255),rgb(176, 255, 255)};

		Mix_Music* mainBGM=Mix_LoadMUS("sounds/main.ogg");
		Mix_Chunk *rain_drop=Mix_LoadWAV("sounds/rain_drop.wav");

		Timer fps;

		int alurenNum=0; //用來紀錄現在的數字鍵值，-1代表按的是數字鍵以外的鍵
		int twinkle=1; //用來當alurens切換子圖的延遲計數器
		float alurensAngle=0; //用來記錄角度
		float alurensAlphaTheta=0,alurensColorTheta=0.02,titleAlphaTheta=0,title2AlphaTheta=0,eggTheta=0,tipsAlphaTheta=0; //用來控制SHM

		SDL_Event e;
		SDL_Event trash_e;

		bool slide = false,fadeInOut=true,first=true,firstTitle=true; //後者記錄alurens的變子圓的淡出淡入
		float slide_down = 0;
		int fade_out = 0;
		Mix_PlayMusic( mainBGM, -1 ); //-1是loop次數，在此是loop到halt(stop)為止
		////////////////////////////////////////////////////////////////////迴圈開始
		while (!quit && !skip)
		{
			fps.start();

			while(SDL_PollEvent(&e) && !slide)
			{
				if(e.type==SDL_QUIT) //單擊右上角的X
				{
					quit=true;
				}
				if(e.type==SDL_KEYDOWN)
				{
					switch(e.key.keysym.sym)
					{
						//ESC退出
						case SDLK_ESCAPE:
							quit=true;
							break;
						case SDLK_TAB:
							skip=true;
							break;
						case SDLK_1:
							level=1;
							skip=true;
							break;
						case SDLK_2:
							level=2;
							skip=true;
							break;
						case SDLK_3:
							level=3;
							skip=true;
							break;
						}
				}
				if(e.type==SDL_MOUSEBUTTONUP && slide_down>3.14 )
				{
					Mix_FadeOutMusic(1200);
					slide = true;
				}
				if(e.type==SDL_MOUSEMOTION && slide_down>3.14 ) //滑鼠移動
					alurensAngle+=e.motion.xrel*0.1+e.motion.yrel*0.1;
			}
			alurensAngle+=0.25;
			Window::clear();

			bg.draw(-120,SHM(slide_down,-360,-180,0.01));

			if (slide_down>2.75) //畫魔法陣
			{
				if(first)
				{
					alurens.setAlpha(SHM(alurensAlphaTheta,255,0,0.016));
					tips.setAlpha(SHM(tipsAlphaTheta,255,0,0.016));
					if(alurensAlphaTheta>3.14)
					{
						alurensAlphaTheta=0;
						tipsAlphaTheta=0.01;
						first=false;
					}
				}
				else
				{
					if(tipsAlphaTheta==0)
					{
						twinkle++;
						alurensColorTheta+=0.02;
					}

					if(twinkle%3==0)
					{
						if(fadeInOut)
						{
							if(alurensAlphaTheta>3.14) //趁完全fade out的時候趕快換色換圖騰
							{
								if(alurenNum<3)
									alurenNum++;
								else
									alurenNum=0;

								alurensAlphaTheta=0; //給接下來的單程SHM fade in用
								tipsAlphaTheta=0.01;
								fadeInOut=false;
							}
							else
							{
								alurens.setAlpha(SHM(alurensAlphaTheta,0,255,0.04));
								tips.setAlpha(SHM(tipsAlphaTheta,0,255,0.04));
							}
						}
						else //fade in
						{
							alurens.setAlpha(SHM(alurensAlphaTheta,255,0,0.04));
							tips.setAlpha(SHM(tipsAlphaTheta,255,0,0.04));
							if(alurensAlphaTheta>3.14)
							{
								alurensAlphaTheta=0;
								tipsAlphaTheta=0.01;
								fadeInOut=true;
								twinkle++;
							}
						}
					}
					else
					{
						tips.setAlpha(SHM(tipsAlphaTheta,80,255,0.04,true));
						switch(alurenNum)
			            {
			                case 0:
								alurensColor[alurenNum].g=SHM(alurensColorTheta,160,128,0.04,true);
	                    		break;
                			case 1:
								alurensColor[alurenNum].b=SHM(alurensColorTheta,240,128,0.04,true);
								break;
                			case 2:
								alurensColor[alurenNum].r=SHM(alurensColorTheta,164,211,0.04,true);
                    			break;
                			case 3:
								alurensColor[alurenNum].r=SHM(alurensColorTheta,240,176,0.04,true);
                    			break;
            			}
					}
				}
				alurens.setColor(alurensColor[alurenNum]);
				tips.setColor(alurensColor[alurenNum]);
				tips.draw(Window::state().w/2,644);
				alurens.draw(Window::state().w/2,Window::state().h/2+80,alurenNum,alurensAngle); //繪製在螢幕正中間，切換到相對應的子圖，旋轉alurensAngle度
			}

			if(firstTitle)
			{
				title.setAlpha(SHM(titleAlphaTheta,255,0,0.007)); //透明度的簡諧運動
				if (finished && slide_down>3.14)
				{
					egg.setAlpha(SHM(eggTheta,255,0,0.01));
					egg2.setAlpha(SHM(eggTheta,255,0,0.01));
				}
				if(slide_down>1.57)
					title2.setAlpha(SHM(title2AlphaTheta,255,0,0.01));
				else
					title2.setAlpha(0);
				if(title2AlphaTheta>3.14)
					firstTitle=false;
			}
			else
			{
				title.setAlpha(SHM(titleAlphaTheta,255,80,0.02,true)); //兩者統一
				title2.setAlpha(SHM(titleAlphaTheta,255,80,0.02,true));
				if (finished)
				{
					egg.setAlpha(SHM(eggTheta,255,80,0.02,true));
					egg2.setAlpha(SHM(eggTheta,255,80,0.02,true));
				}
			}

			title.draw(Window::state().w/2,180);
			title2.draw(Window::state().w/2,244);
			if (finished)
			{
				egg.draw(475-egg.getWidth(),752);
				egg2.draw(475-egg2.getWidth(),775);
			}

			if(fps.ticks()*FPS<1000)
				SDL_Delay((1000/FPS)- fps.ticks());

			if (slide)
			{
				fade_out++;
				black->setAlpha(fade_out*3);
				black->draw(0,0);
				if (fade_out == 1)
					Mix_PlayChannel(-1,rain_drop,0);
				else if (fade_out == 85)
				{
					SDL_Delay(1000);
					break;
				}
			}

			Window::present();
		}
		Window::quit();
		delete black;
		if (quit)
			exit(0);
		skip=false;
		///////////////////////////////////////////////切換橫視窗
		slide = false;
		Window::initialize_wide("Eschatology");

		Texture bg1("img/bg1.png");
		Texture bg2("img/bg2.png");
		Texture bg3("img/bg3.png");
		Texture* black_w = new Texture("img/black_w.png");
		Texture* chat = new Texture("img/chat.png");
		Texture* face1 = new Texture("img/face1.png");
		Texture* face1_2 = new Texture("img/face1_2.png");
		Texture face2("img/face2.png");
		Texture face3("img/face3.png");
		Texture bg_flare("img/bg_flare.png");
		bg_flare.setDstRect(0,0,{20,0});

		mainBGM = Mix_LoadMUS("sounds/starting.ogg");
		Mix_Chunk *blow = Mix_LoadWAV("sounds/blow.wav");
		Mix_Chunk *transition = Mix_LoadWAV("sounds/transition.wav");
		Mix_PlayMusic(mainBGM,-1);

		string talking[39];
		talking[0] = "報告，已經確認黑盒子反應";
		talking[1] = "目標：奪取黑盒子並消滅目擊相關人士";
		talking[2] = "進行最後確認";
		talking[3] = "北方";
		talking[4] = "無異狀";
		talking[5] = "南方";
		talking[6] = "無異狀";
		talking[7] = "東方";
		talking[8] = "無異狀";
		talking[9] = "西方";
		talking[10] = "無異狀";
		talking[11] = "確認完畢，倒數十秒";
		talking[12] = " 10！";
		talking[13] = "！！！";
		talking[14] = "居然不知不覺睡著了......";
		talking[15] = "已經很晚了啊，不知道爸睡了沒？";
		talking[16] = "去樓下看看好了";
		talking[17] = "......！";
		talking[18] = "周圍魔力的流動很詭異，發生了什麼事？";
		talking[19] = "爸！你還醒著嗎？";
		talking[20] = "羅雷，快趴下！";
		talking[21] = "３！";
		talking[22] = "２！";
		talking[23] = "１！";
		talking[24] = "發動，地獄業火！";
		talking[25] = "";
		talking[26] = "這種程度的攻擊魔法...敵人究竟有多少？";
		talking[27] = "別管那麼多了，羅雷，帶著黑盒子快跑！";
		talking[28] = "跑？去哪裡？那你呢？";
		talking[29] = "往南邊跑！米亞姊會去接應你的。我留下來拖延他們的腳步";
		talking[30] = "別開玩笑了！我怎麼可能眼睜睜看著你...";
		talking[31] = "我們沒有選擇！";
		talking[32] = "敵人的目標是黑盒子，這是我們的使命";
		talking[33] = "我......";
		talking[34] = "動作快！";
		talking[35] = "......";
		talking[36] = "可惡！";
		talking[37] = "我打開藏著黑盒子的暗門，帶著黑盒子從後門跑出去，身後敵人源源不絕的湧上來";
		talking[38] = "黑盒子在那個少年身上，別讓他跑了！";
		int word_count[39] = {12,17,6,2,3,2,3,2,3,2,3,9,2,3,11,15,7,3,18,8,7,2,2,2,8,0,18,18,10,26,18,7,17,3,4,2,3,36,17};

		int talk = 0;
		int display = 0;
		bool display_end = false;
		int shake = 0;
		int direction = -1;
		int shake_times = 0;
		int alpha = 0;
		int cls_pos = 0;
		while (!quit && !skip)
		{
			fps.start();

			if (talk < 39)
			{
				display_end = false;
				display++;
				if (display > word_count[talk] * 3)
				{
					display = word_count[talk] * 3;
					display_end = true;
				}
			}
			while(SDL_PollEvent(&e))
			{
				if(e.type==SDL_QUIT) //單擊右上角的X
				{
					quit=true;
				}
				if(e.type==SDL_KEYDOWN)
				{
					switch(e.key.keysym.sym)
					{
						//ESC退出
						case SDLK_ESCAPE:
							quit=true;
							break;
						case SDLK_TAB:
							skip=true;
							break;
					}
				}
				if(e.type==SDL_MOUSEBUTTONUP && talk != 25)
				{
					if (!display_end)
					{
						display = word_count[talk] * 3;
						display_end = true;
					}
					else
					{
						if (talk < 39)
							talk++;
						if (talk == 25)
							Mix_PlayChannel(-1,blow,0);
						else if (talk == 13)
							alpha = 255;
						else if (talk == 14)
							black_w->setAlpha(255);
						else if (talk == 39 && cls_pos == 0)
						{
							Mix_PlayChannel(-1,transition,0);
							Mix_FadeOutMusic(600);
						}
						display = 0;
					}
				}
			}
			Window::clear();
			if (talk >= 13 && talk <= 20)
			{
				bg1.draw(0,0);
				if (talk == 13)
				{
					black_w->setAlpha(alpha);
					black_w->draw(0,0);
					if (alpha > 0)
						alpha -= 51;
				}
			}
			else if (talk == 25)
			{
				shake += direction * 20;
				shake_times++;
				if (shake == -20)
					direction = 1;
				if (shake == 20)
					direction = -1;
				if (shake_times == 35)
				{
					talk = 26;
					bg2.draw(0,0);
					Window::present();
					SDL_Delay(2000);
					while (SDL_PollEvent(&trash_e));
					continue;
				}
				bg_flare.draw(shake,0);
			}
			else if (talk >= 26 && talk <= 36)
				bg2.draw(0,0);
			else if (talk >= 37)
				bg3.draw(0,0);

			if (talk >= 13 && talk != 21 && talk != 22 && talk != 23 && talk != 24)
				chat->draw(0,320);
			////////////////////////////////////////表情
			if ((talk >= 13 && talk <= 16) || talk == 26 || talk == 28 || talk == 33 || talk == 35)
				face1->draw(0,320);
			else if (talk == 17 || talk == 18 || talk == 19 || talk == 30 || talk == 36)
				face1_2->draw(0,320);
			else if (talk == 20 || talk == 27 || talk == 29 || talk == 31 || talk == 32 || talk == 34)
				face2.draw(0,320);
			else if (talk == 38)
				face3.draw(0,320);
			////////////////////////////////////////表情
			if (talk < 39)
			{
				if (display <= 54)
				{
					Texture* temp = new Texture(talking[talk].substr(0,(display/3)*3),rgb(255, 255, 255),24);
					temp->setDstRect(0,0,temp->setPoint());
					if (talk < 13 || talk == 21 || talk == 22 || talk == 23 || talk == 24 || talk == 37)
						temp->draw(400,400);
					else
						temp->draw(460,400);
					delete temp;
				}
				else
				{
					Texture* temp = new Texture(talking[talk].substr(0,54),rgb(255, 255, 255),24);
					temp->setDstRect(0,0,temp->setPoint());
					if (talk < 13 || talk == 21 || talk == 22 || talk == 23 || talk == 24 || talk == 37)
						temp->draw(400,376);
					else
						temp->draw(460,376);
					Texture* temp2 = new Texture(talking[talk].substr(54,(display/3)*3-54),rgb(255, 255, 255),24);
					temp2->setDstRect(0,0,temp2->setPoint());
					if (talk < 13 || talk == 21 || talk == 22 || talk == 23 || talk == 24 || talk == 37)
						temp2->draw(400,424);
					else
						temp2->draw(460,424);
					delete temp;
					delete temp2;
				}
			}
			else if (talk == 39)
			{
				cls_pos += 20;
				black_w->draw(-800+cls_pos,0);
				if (cls_pos == 820)
				{
					SDL_Delay(200);
					cls_pos = 0;
					break;
				}
			}

			if (fps.ticks()*FPS<1000)
				SDL_Delay((1000/FPS)- fps.ticks());
			Window::present();
		}
		Window::quit();
		delete black_w;
		delete chat;
		delete face1;
		delete face1_2;
		if (quit)
			exit(0);
		skip=false;
		///////////////////////////////////////////////切換戰鬥畫面
		Window::initialize("Eschatology");

		black = new Texture("img/black.png");
		Texture minion("img/minion.png");
		Texture* character1 = new Texture("img/character1.png");
		Texture soldier("img/soldier.png");
		Texture soldier2("img/soldier2.png");
		Texture* hp_ground = new Texture("img/hp_ground.png");
		Texture* hp_layer = new Texture("img/hp_layer.png");
		Texture* hp2_ground = new Texture("img/hp2_ground.png");
		Texture* hp2_layer = new Texture("img/hp2_layer.png");
		Texture* aluren = new Texture("img/aluren.png");
		Texture* aluren_click = new Texture("img/aluren_click.png");
		Texture* temp_circle = new Texture("img/circle.png");
		Texture* buttonFlame = new Texture("img/buttonFlame.png");
		Texture* buttonFlame_p = new Texture("img/buttonFlame_p.png");
		Texture* buttonIce = new Texture("img/buttonIce.png");
		Texture* buttonIce_p = new Texture("img/buttonIce_p.png");
		Texture* buttonLight = new Texture("img/buttonLight.png");
		Texture* buttonLight_p = new Texture("img/buttonLight_p.png");
		Texture* buttonDark = new Texture("img/buttonDark.png");
		Texture* buttonDark_p = new Texture("img/buttonDark_p.png");
		Texture* battle_bg = new Texture("img/battle_bg.png");
		Texture full_hp("100/100",rgb(255, 255, 255),16);
		Texture full_hp2("100/100",rgb(255, 255, 255),16);
		Texture** flame = new Texture* [6];
		Texture** ice = new Texture* [6];
		Texture** light = new Texture* [6];
		Texture** dark = new Texture* [6];
		Texture** spear = new Texture* [6];
		Texture** slash = new Texture* [6];
		Texture** bad_click = new Texture* [6];
		string anime_frame[6] = {"1.png","2.png","3.png","4.png","5.png","6.png"};
		for (int i = 0; i < 6; i++)
		{
			string temp_name;
			temp_name = "img/anime/flame"+anime_frame[i];
			flame[i] = new Texture(temp_name);
			temp_name = "img/anime/ice"+anime_frame[i];
			ice[i] = new Texture(temp_name);
			temp_name = "img/anime/light"+anime_frame[i];
			light[i] = new Texture(temp_name);
			temp_name = "img/anime/dark"+anime_frame[i];
			dark[i] = new Texture(temp_name);
			temp_name = "img/anime/spear"+anime_frame[i];
			spear[i] = new Texture(temp_name);
			temp_name = "img/anime/slash"+anime_frame[i];
			slash[i] = new Texture(temp_name);
			temp_name = "img/anime/bad_click"+anime_frame[i];
			bad_click[i] = new Texture(temp_name);
		}
		Mix_Chunk *transition_r = Mix_LoadWAV("sounds/transition_r.wav");
		Mix_Chunk *flame_s = Mix_LoadWAV("sounds/flame.wav");
		Mix_Chunk *ice_s = Mix_LoadWAV("sounds/ice.wav");
		Mix_Chunk *light_s = Mix_LoadWAV("sounds/light.wav");
		Mix_Chunk *dark_s = Mix_LoadWAV("sounds/dark.wav");
		Mix_Chunk *spear_s = Mix_LoadWAV("sounds/spear.wav");
		Mix_Chunk *slash_s = Mix_LoadWAV("sounds/slash.wav");
		Mix_Chunk *battle_pre = Mix_LoadWAV("sounds/battle_pre.ogg");
		mainBGM = Mix_LoadMUS("sounds/battle1.ogg");
		SDL_Delay(500);
		Mix_PlayChannel(-1,transition_r,0);

		const double BEAT_DURATION = displayedBeats;
		Timer attack;

		display = 0;
		cls_pos = 0;
		string battle_text = "一些士兵擋住了去路";
		string battle_text1_1 = "隨著節奏抓準魔力的流動";
		string battle_text1_2 = "定時或漏掉魔法波時你會遭受攻擊";
		string battle_text2_1 = "自由使用按鍵 E F I J ";
		string battle_text2_2 = "對應「火冰光闇」進行攻擊";
		string battle_text2_3 = "波到達魔法陣邊緣時就是最佳時機";
		string battle_text2_4 = "依精準度亮起白、黃、綠、靛";
		string battle_text3_1 = "你的攻擊力隨著精準度提升";
		string battle_text3_2 = "不精準導致亮起暗紫時，你將遭魔法反嗜";

		string battle_text4 = "...是魔法護甲！";
		string battle_text5 = "依序組合［火冰火光暗］施展雷槍";
		int hp1 = 100;
		int hp2 = 100;
		vector<int> flame_anime;
		vector<int> ice_anime;
		vector<int> light_anime;
		vector<int> dark_anime;
		vector<int> spear_anime;
		vector<int> slash_anime;
		vector<pair<int,SDL_Color>> bad_click_anime;
		int skill = 0;
		int stage = 0;
		int click_effect = 0;
		bool in_battle = false;
		ifstream song("charts/battle1/battle1-E.jc",ifstream::in);
		ChartPlayer* chart = new ChartPlayer(song);
		vector<Judgement> judge;
		int last_judge = 0;
		int damage = 10;
		int e_damage = 1;
		double resist = 1;
		double damage_rate[4] = {1,0.9,0.7,0.4};
		SDL_Color accuracy[5] = {rgb(255, 255, 255),rgb(255, 255, 0),rgb(128, 255, 128),rgb(0, 196, 196),rgb(0, 0, 128)};
		bool pressing[4] = {false,false,false,false};
		Mix_PlayChannel(-1,spear_s,0);
		fps.start();
		replay = false;
		while (!quit && !skip)
		{
			if (replay)
			{
				delete chart;
				song.clear();
				song.seekg(0,ios::beg);
				Mix_PlayMusic(mainBGM,0);
				chart = new ChartPlayer(song);
				chart->start();
				replay = false;
			}

			if (in_battle)
			{
				judge = chart->getJudgements();
				for (int i = last_judge; i < judge.size(); i++)
				{
					if (judge[i] == MISS)
					{
						slash_anime.push_back(0);
						Mix_PlayChannel(-1,slash_s,0);
						hp1 -= e_damage;
						skill = 0;
					}
				}
				last_judge = judge.size();
			}

			if (attack.ticks() > 8000 && in_battle)
			{
				slash_anime.push_back(0);
				Mix_PlayChannel(-1,slash_s,0);
				hp1 -= e_damage;
				attack.restart();
			}

			while(SDL_PollEvent(&e))
			{
				if(e.type==SDL_QUIT) //單擊右上角的X
				{
					quit=true;
				}
				if(e.type==SDL_KEYDOWN)
				{
					if (e.key.keysym.sym == SDLK_ESCAPE)
					{
						//ESC退出
						quit=true;
						break;
					}
					else if(e.key.keysym.sym == SDLK_TAB)
					{
						skip=true;
						break;
					}
					else if (e.key.keysym.sym == SDLK_f || e.key.keysym.sym == SDLK_e || e.key.keysym.sym == SDLK_j || e.key.keysym.sym == SDLK_i)
					{
						if (in_battle)
						{
							switch (e.key.keysym.sym)
							{
								case SDLK_e:
									pressing[0] = true;
									break;
								case SDLK_f:
									pressing[1] = true;
									break;
								case SDLK_i:
									pressing[2] = true;
									break;
								case SDLK_j:
									pressing[3] = true;
									break;
							}
							chart->hit();
							judge = chart->getJudgements();
							for (int i = last_judge; i < judge.size(); i++)
							{
								if (judge[i] == MISS)
								{
									slash_anime.push_back(0);
									Mix_PlayChannel(-1,slash_s,0);
									hp1 -= e_damage;
									skill = 0;
								}
								else if (judge[i] != WRONG)
								{
									click_effect = 1;
									aluren_click->setColor(accuracy[judge[i]]);
									switch (e.key.keysym.sym)
									{
										case SDLK_e:
											Mix_PlayChannel(-1,flame_s,0);
											if (judge[i] == 4)
											{
												bad_click_anime.push_back(make_pair(0,rgb(255,0,0)));
												hp1 -= e_damage/2;
												skill = 0;
											}
											else
											{
												flame_anime.push_back(0);
												hp2 -= damage*damage_rate[judge[i]]*resist;
												if (stage == 1)
												{
													if (skill == 0 || skill == 2)
														skill++;
													else
														skill = 0;
												}
											}
											break;
										case SDLK_f:
											Mix_PlayChannel(-1,ice_s,0);
											if (judge[i] == 4)
											{
												bad_click_anime.push_back(make_pair(0,rgb(130,245,255)));
												hp1 -= e_damage/2;
												skill = 0;
											}
											else
											{
												ice_anime.push_back(0);
												hp2 -= damage*damage_rate[judge[i]]*resist;
												if (stage == 1)
												{
													if (skill == 1)
														skill++;
													else
														skill = 0;
												}
											}
											break;
										case SDLK_i:
											Mix_PlayChannel(-1,light_s,0);
											if (judge[i] == 4)
											{
												bad_click_anime.push_back(make_pair(0,rgb(255,255,130)));
												hp1 -= e_damage/2;
												skill = 0;
											}
											else
											{
												light_anime.push_back(0);
												hp2 -= damage*damage_rate[judge[i]]*resist;
												if (stage == 1)
												{
													if (skill == 3)
														skill++;
													else
														skill = 0;
												}
											}
											break;
										case SDLK_j:
											if(stage==0)
											{
												Mix_PlayChannel(-1,dark_s,0);
												if (judge[i] != 4)
													dark_anime.push_back(0);
											}
											if (judge[i] == 4)
											{
												bad_click_anime.push_back(make_pair(0,rgb(70,0,115)));
												hp1 -= e_damage/2;
											}
											else
											{
												hp2 -= damage*damage_rate[judge[i]]*resist;
												if (stage == 1)
												{
													if (skill == 4)
													{
														hp2 -= 20;
														spear_anime.push_back(0);
														Mix_PlayChannel(-1,spear_s,0);
													}
													else
													{
														Mix_PlayChannel(-1,dark_s,0);
														dark_anime.push_back(0);
													}
												}
											}
											skill = 0;
											break;
									}
								}
							}
							last_judge = judge.size();
						}
					}
				}
				if(e.type==SDL_KEYUP)
				{
					switch (e.key.keysym.sym)
					{
						case SDLK_e:
							pressing[0] = false;
							break;
						case SDLK_f:
							pressing[1] = false;
							break;
						case SDLK_i:
							pressing[2] = false;
							break;
						case SDLK_j:
							pressing[3] = false;
							break;
					}
				}
			}
			if (fps.ticks() > 16)
			{
				fps.restart();
				Window::clear();
				minion.draw(0,0);
				character1->draw(0,0);
				battle_bg->draw(0,180);
				if (pressing[0])
					buttonFlame_p->draw(16,589);
				else
					buttonFlame->draw(16,589);
				if (pressing[1])
					buttonIce_p->draw(98,678);
				else
					buttonIce->draw(98,678);
				if (pressing[2])
					buttonLight_p->draw(374,589);
				else
					buttonLight->draw(374,589);
				if (pressing[3])
					buttonDark_p->draw(292,678);
				else
					buttonDark->draw(292,678);
				if (stage == 0)
				{
					soldier.draw(240,0);
					if (display <= 27)
					{
						Texture* temp = new Texture(battle_text.substr(0,(display/3)*3),rgb(255, 255, 255),24);
						temp->setDstRect(0,0,temp->setPoint());
						temp->draw(240,300);
						delete temp;
					}
					else if (display <= 60)
					{
						Texture* temp = new Texture(battle_text,rgb(255, 255, 255),24);
						Texture* temp2 = new Texture(battle_text1_1.substr(0,((display-27)/3)*3),rgb(255, 255, 255),24);
						temp->setDstRect(0,0,temp->setPoint());
						temp2->setDstRect(0,0,temp2->setPoint());
						temp->draw(240,300);
						temp2->draw(240,336);
						delete temp;
						delete temp2;
					}
					else if (display <= 105)
					{
						Texture* temp = new Texture(battle_text,rgb(255, 255, 255),24);
						Texture* temp2 = new Texture(battle_text1_1,rgb(255, 255, 255),24);
						Texture* temp3 = new Texture(battle_text1_2.substr(0,((display-60)/3)*3),rgb(255, 255, 255),24);
						temp->setDstRect(0,0,temp->setPoint());
						temp2->setDstRect(0,0,temp2->setPoint());
						temp3->setDstRect(0,0,temp3->setPoint());
						temp->draw(240,300);
						temp2->draw(240,336);
						temp3->draw(240,372);
						delete temp;
						delete temp2;
						delete temp3;
					}
					else if (display > 105 && display < 180)
					{
						Texture* temp = new Texture(battle_text,rgb(255, 255, 255),24);
						Texture* temp2 = new Texture(battle_text1_1,rgb(255, 255, 255),24);
						Texture* temp3 = new Texture(battle_text1_2,rgb(255, 255, 255),24);
						temp->setDstRect(0,0,temp->setPoint());
						temp2->setDstRect(0,0,temp2->setPoint());
						temp3->setDstRect(0,0,temp3->setPoint());

						temp->draw(240,300);
						temp2->draw(240,336);
						temp3->draw(240,372);
						delete temp;
						delete temp2;
						delete temp3;
					}
					else if (display <= 207)
					{
						Texture* temp = new Texture(battle_text2_1.substr(0,((display-180)/3)*3),rgb(255, 255, 255),24);
						temp->setDstRect(0,0,temp->setPoint());
						temp->draw(240,300);
						delete temp;
					}
					else if (display <= 243)
					{
						Texture* temp = new Texture(battle_text2_1,rgb(255, 255, 255),24);
						Texture* temp2 = new Texture(battle_text2_2.substr(0,((display-207)/3)*3),rgb(255, 255, 255),24);
						temp->setDstRect(0,0,temp->setPoint());
						temp2->setDstRect(0,0,temp2->setPoint());
						temp->draw(240,300);
						temp2->draw(240,336);
						delete temp;
						delete temp2;
					}
					else if (display <= 288)
					{
						Texture* temp = new Texture(battle_text2_1,rgb(255, 255, 255),24);
						Texture* temp2 = new Texture(battle_text2_2,rgb(255, 255, 255),24);
						Texture* temp3 = new Texture(battle_text2_3.substr(0,((display-243)/3)*3),rgb(255, 255, 255),24);
						temp->setDstRect(0,0,temp->setPoint());
						temp2->setDstRect(0,0,temp2->setPoint());
						temp3->setDstRect(0,0,temp3->setPoint());
						temp->draw(240,300);
						temp2->draw(240,336);
						temp3->draw(240,372);
						delete temp;
						delete temp2;
						delete temp3;
					}
					else if (display <= 327)
					{
						Texture* temp = new Texture(battle_text2_1,rgb(255, 255, 255),24);
						Texture* temp2 = new Texture(battle_text2_2,rgb(255, 255, 255),24);
						Texture* temp3 = new Texture(battle_text2_3,rgb(255, 255, 255),24);
						Texture* temp4 = new Texture(battle_text2_4.substr(0,((display-288)/3)*3),rgb(255, 255, 255),24);
						temp->setDstRect(0,0,temp->setPoint());
						temp2->setDstRect(0,0,temp2->setPoint());
						temp3->setDstRect(0,0,temp3->setPoint());
						temp4->setDstRect(0,0,temp4->setPoint());
						temp->draw(240,300);
						temp2->draw(240,336);
						temp3->draw(240,372);
						temp4->draw(240,408);
						delete temp;
						delete temp2;
						delete temp3;
						delete temp4;
					}
					else if (display > 327 && display < 417)
					{
						Texture* temp = new Texture(battle_text2_1,rgb(255, 255, 255),24);
						Texture* temp2 = new Texture(battle_text2_2,rgb(255, 255, 255),24);
						Texture* temp3 = new Texture(battle_text2_3,rgb(255, 255, 255),24);
						Texture* temp4 = new Texture(battle_text2_4,rgb(255, 255, 255),24);
						temp->setDstRect(0,0,temp->setPoint());
						temp2->setDstRect(0,0,temp2->setPoint());
						temp3->setDstRect(0,0,temp3->setPoint());
						temp4->setDstRect(0,0,temp4->setPoint());
						//temp->setAlpha(240-(display-282)*8);
						//temp2->setAlpha(240-(display-282)*8);
						//temp3->setAlpha(240-(display-282)*8);
						//temp4->setAlpha(240-(display-282)*8);
						temp->draw(240,300);
						temp2->draw(240,336);
						temp3->draw(240,372);
						temp4->draw(240,408);
						delete temp;
						delete temp2;
						delete temp3;
						delete temp4;
					}
					else if (display <= 453)
					{
						Texture* temp = new Texture(battle_text3_1.substr(0,((display-417)/3)*3),rgb(255, 255, 255),24);
						temp->setDstRect(0,0,temp->setPoint());
						temp->draw(240,300);
						delete temp;
					}
					else if (display <= 507)
					{
						Texture* temp = new Texture(battle_text3_1,rgb(255, 255, 255),24);
						Texture* temp2 = new Texture(battle_text3_2.substr(0,((display-453)/3)*3),rgb(255, 255, 255),24);
						temp->setDstRect(0,0,temp->setPoint());
						temp2->setDstRect(0,0,temp2->setPoint());
						temp->draw(240,300);
						temp2->draw(240,336);
						delete temp;
						delete temp2;
					}
					else if (display > 507 && display < 600)
					{
						Texture* temp = new Texture(battle_text3_1,rgb(255, 255, 255),24);
						Texture* temp2 = new Texture(battle_text3_2,rgb(255, 255, 255),24);
						temp->setDstRect(0,0,temp->setPoint());
						temp2->setDstRect(0,0,temp2->setPoint());
						if(display > 570)
						{
							temp->setAlpha(240-(display-570)*8);
							temp2->setAlpha(240-(display-570)*8);
						}
						temp->draw(240,300);
						temp2->draw(240,336);
						delete temp;
						delete temp2;
					}

					if (display < 600 && cls_pos == 800)
					{
						display++;
						if (display == 334)
						{
							Mix_PlayMusic(mainBGM,0);
							Mix_HookMusicFinished(replayBGM);
							chart->start();
							attack.start();
						}
						if (display > 540 && display <= 570)
						{
							hp_layer->draw((display-540)*8-240,180);
							hp2_layer->draw(480-(display-540)*8,180);
							full_hp.setAlpha((display-540)*8+15);
							full_hp2.setAlpha((display-540)*8+15);
							full_hp.draw(15,195);
							full_hp2.draw(465-full_hp2.getWidth(),195);
						}
						else if (display > 570 && display < 600)
						{
							hp_layer->draw(0,180);
							hp2_layer->draw(240,180);
							full_hp.draw(15,195);
							full_hp2.draw(465-full_hp2.getWidth(),195);
							aluren->setAlpha((display-570)*8+15);
							aluren->draw(90,270);
						}
					}
					if (display == 600)
					{
						in_battle = true;
						cls_pos = 0;
						hp_ground->draw(0,180);
						hp2_ground->draw(240,180);
						hp_layer->draw(hp1*240/100-240,180);
						hp2_layer->draw(480-hp2*240/100,180);
						stringstream ss;
						stringstream ss2;
						string temp_hp;
						ss << hp1;
						ss >> temp_hp;
						Texture* hp1_t = new Texture(temp_hp+"/100",rgb(255, 255, 255),16);
						ss2 << hp2;
						ss2 >> temp_hp;
						Texture* hp2_t = new Texture(temp_hp+"/100",rgb(255, 255, 255),16);
						hp1_t->draw(15,195);
						hp2_t->draw(465-hp2_t->getWidth(),195);
						aluren->draw(90,270);
					}
				}
				else if (stage == 1)
				{
					if (display < 510)
					{
						display++;
						if (display < 30)
						{
							soldier.setAlpha(240-display*8);
							soldier.draw(240,0);
							aluren->setAlpha(240-display*8);
							aluren->draw(90,270);
							hp_ground->draw(0,180);
							hp2_ground->draw(240,180);
							hp_layer->draw(hp1*240/100-240,180);
							stringstream ss;
							string temp_hp;
							ss << hp1;
							ss >> temp_hp;
							Texture* hp1_t = new Texture(temp_hp+"/100",rgb(255, 255, 255),16);
							Texture* hp2_t = new Texture("0/100",rgb(255, 255, 255),16);
							hp1_t->draw(15,195);
							hp2_t->draw(465-hp2_t->getWidth(),195);
						}
						else if (display < 90)
						{
							hp2++;
							soldier2.draw(600-(display-30)*6,0);
							hp_ground->draw(0,180);
							hp2_ground->draw(240,180);
							hp_layer->draw(hp1*240/100-240,180);
							hp2_layer->draw(480-(display-30)*4,180);
							stringstream ss;
							stringstream ss2;
							string temp_hp;
							ss << hp1;
							ss >> temp_hp;
							Texture* hp1_t = new Texture(temp_hp+"/100",rgb(255, 255, 255),16);
							ss2 << hp2;
							ss2 >> temp_hp;
							Texture* hp2_t = new Texture(temp_hp+"/100",rgb(255, 255, 255),16);
							hp1_t->draw(15,195);
							hp2_t->draw(465-hp2_t->getWidth(),195);
							if (display <= 51)
							{
								Texture* temp = new Texture(battle_text4.substr(0,((display-30)/3)*3),rgb(255, 255, 255),24);
								temp->setDstRect(0,0,temp->setPoint());
								temp->draw(240,300);
								delete temp;
							}
							else if (display < 90)
							{
								Texture* temp = new Texture(battle_text4.substr(0,21),rgb(255, 255, 255),24);
								temp->setDstRect(0,0,temp->setPoint());
								temp->draw(240,300);
								Texture* temp2 = new Texture(battle_text5.substr(0,((display-51)/3)*3),rgb(255, 255, 255),24);
								temp2->setDstRect(0,0,temp2->setPoint());
								temp2->draw(240,336);
								delete temp;
								delete temp2;
							}
						}
						else if (display >= 90)
						{
							soldier2.draw(240,0);
							hp_ground->draw(0,180);
							hp2_ground->draw(240,180);
							hp_layer->draw(hp1*240/100-240,180);
							hp2_layer->draw(240,180);
							stringstream ss;
							string temp_hp;
							ss << hp1;
							ss >> temp_hp;
							Texture* hp1_t = new Texture(temp_hp+"/100",rgb(255, 255, 255),16);
							Texture* hp2_t = new Texture("100/100",rgb(255, 255, 255),16);
							hp1_t->draw(15,195);
							hp2_t->draw(465-hp2_t->getWidth(),195);
							if (display < 480)
							{
								Texture* temp = new Texture(battle_text4.substr(0,21),rgb(255, 255, 255),24);
								temp->setDstRect(0,0,temp->setPoint());
								if (display > 450)
									temp->setAlpha(240-(display-450)*8);
								temp->draw(240,300);
								delete temp;
								if (display < 96)
								{
									Texture* temp2 = new Texture(battle_text5.substr(0,((display-51)/3)*3),rgb(255, 255, 255),24);
									temp2->setDstRect(0,0,temp2->setPoint());
									temp2->draw(240,336);
									delete temp2;
								}
								else
								{
									Texture* temp2 = new Texture(battle_text5.substr(0,45),rgb(255, 255, 255),24);
									temp2->setDstRect(0,0,temp2->setPoint());
									if (display > 450)
										temp2->setAlpha(240-(display-450)*8);
									temp2->draw(240,336);
									delete temp2;
								}
							}
							else
							{
								aluren->setAlpha(15+(display-480)*8);
								aluren->draw(90,270);
							}
						}
						if (display == 510)
							in_battle = true;
					}
					else if (display == 510)
					{
						soldier2.draw(240,0);
						hp_ground->draw(0,180);
						hp2_ground->draw(240,180);
						hp_layer->draw(hp1*240/100-240,180);
						hp2_layer->draw(480-hp2*240/100,180);
						stringstream ss;
						stringstream ss2;
						string temp_hp;
						ss << hp1;
						ss >> temp_hp;
						Texture* hp1_t = new Texture(temp_hp+"/100",rgb(255, 255, 255),16);
						ss2 << hp2;
						ss2 >> temp_hp;
						Texture* hp2_t = new Texture(temp_hp+"/100",rgb(255, 255, 255),16);
						hp1_t->draw(15,195);
						hp2_t->draw(465-hp2_t->getWidth(),195);
						aluren->draw(90,270);
					}
				}
				if (click_effect > 0)
				{
					aluren_click->setAlpha(255-17*click_effect);
					aluren_click->draw(80,260);
					click_effect++;
					if (click_effect == 15)
						click_effect = 0;
				}
				////////////////////////////////////////////////////////譜面顯示
				for (int t = 0; t < 1 && in_battle; t++)
				{
					auto temp_chart = chart->getNotePositions(BEAT_DURATION);
					for (int i = 0; i < temp_chart.size(); i++)
					{
						int temp_r = 300*(BEAT_DURATION-temp_chart.at(i).first)/BEAT_DURATION;
						if (temp_r == 0)
							temp_r = 2;
						temp_circle->setDstRect(temp_r,temp_r);
						temp_circle->draw(240-temp_r/2,420-temp_r/2);
					}
				}
				////////////////////////////////////////////////////////動畫表現
				for (int i = 0; i < flame_anime.size(); i++)
				{
					flame[flame_anime.at(i)/2]->draw(280,0);
					flame_anime.at(i)++;
					if (flame_anime.at(i) == 12)
						flame_anime.erase(flame_anime.begin()+i);
				}
				for (int i = 0; i < ice_anime.size(); i++)
				{
					ice[ice_anime.at(i)/2]->draw(280,0);
					ice_anime.at(i)++;
					if (ice_anime.at(i) == 12)
						ice_anime.erase(ice_anime.begin()+i);
				}
				for (int i = 0; i < light_anime.size(); i++)
				{
					light[light_anime.at(i)/2]->draw(280,0);
					light_anime.at(i)++;
					if (light_anime.at(i) == 12)
						light_anime.erase(light_anime.begin()+i);
				}
				for (int i = 0; i < dark_anime.size(); i++)
				{
					dark[dark_anime.at(i)/2]->draw(280,0);
					dark_anime.at(i)++;
					if (dark_anime.at(i) == 12)
						dark_anime.erase(dark_anime.begin()+i);
				}
				for (int i = 0; i < spear_anime.size(); i++)
				{
					spear[spear_anime.at(i)/2]->draw(280,0);
					spear_anime.at(i)++;
					if (spear_anime.at(i) == 12)
						spear_anime.erase(spear_anime.begin()+i);
				}
				for (int i = 0; i < slash_anime.size(); i++)
				{
					slash[slash_anime.at(i)/2]->draw(0,0);
					slash_anime.at(i)++;
					if (slash_anime.at(i) == 12)
						slash_anime.erase(slash_anime.begin()+i);
				}
				for (int i = 0; i < bad_click_anime.size(); i++)
				{
					bad_click[bad_click_anime.at(i).first/2]->setColor(bad_click_anime.at(i).second);
					bad_click[bad_click_anime.at(i).first/2]->draw(0,0);
					bad_click_anime.at(i).first++;
					if (bad_click_anime.at(i).first == 12)
						bad_click_anime.erase(bad_click_anime.begin()+i);
				}
				/////////////////////////////////////////////////////////////////
				if (cls_pos < 800 && display == 0)
				{
					black->draw(0,cls_pos);
					cls_pos += 20;
					if (cls_pos == 800)
						Mix_PlayChannel(-1,battle_pre,0);
				}
				if (hp1 <= 0)
				{
					in_battle = false;
					black->setAlpha(cls_pos);
					black->draw(0,0);
					cls_pos += 15;
					if (cls_pos == 15)
						Mix_FadeOutMusic(500);
					else if (cls_pos == 270)
					{
						quit = true;
						break;
					}
				}
				if (hp2 <= 0)
				{
					stage++;
					in_battle = false;
					if (stage == 1)
					{
						display = 0;
						cls_pos = 0;
						hp2 = 40;
						resist = 0.1;
					}
					else
					{
						attack.stop();
						black->setAlpha(cls_pos);
						black->draw(0,0);
						cls_pos += 15;
						if (cls_pos == 15)
							Mix_FadeOutMusic(500);
						else if (cls_pos == 270)
						{
							SDL_Delay(500);
							break;
						}
					}
				}

				Window::present();
			}
		}
		Window::quit();
		song.close();
		for (int i = 0; i < 6; i++)
		{
			delete flame[i];
			delete ice[i];
			delete light[i];
			delete dark[i];
			delete spear[i];
			delete slash[i];
			delete bad_click[i];
		}
		delete [] flame;
		delete [] ice;
		delete [] light;
		delete [] dark;
		delete [] spear;
		delete [] slash;
		delete [] bad_click;
		delete black;
		delete battle_bg;
		delete aluren;
		delete aluren_click;
		delete temp_circle;
		delete buttonFlame;
		delete buttonFlame_p;
		delete buttonIce;
		delete buttonIce_p;
		delete buttonLight;
		delete buttonLight_p;
		delete buttonDark;
		delete buttonDark_p;
		delete character1;
		delete hp_ground;
		delete hp_layer;
		delete hp2_ground;
		delete hp2_layer;
		delete chart;
		if (quit)
			exit(0);
		skip=false;
		///////////////////////////////////////////////////////////////切回動畫畫面
		Window::initialize_wide("Eschatology");

		Texture bg4("img/bg4.png");
		chat = new Texture("img/chat.png");
		black_w = new Texture("img/black_w.png");
		Texture** big_fire = new Texture* [6];
		for (int i = 0; i < 6; i++)
		{
			string temp_name;
			temp_name = "img/anime/fire"+anime_frame[i];
			big_fire[i] = new Texture(temp_name);
		}
		face1 = new Texture("img/face1.png");
		face1_2 = new Texture("img/face1_2.png");
		Texture* face4 = new Texture("img/face4.png");

		mainBGM = Mix_LoadMUS("sounds/herius.ogg");
		transition = Mix_LoadWAV("sounds/transition.wav");
		Mix_Chunk *fire = Mix_LoadWAV("sounds/fire.wav");
		Mix_PlayMusic(mainBGM,-1);

		string talking2[9];
		talking2[0] = "前面就是森林了，應該可以甩掉他們！";
		talking2[1] = "";
		talking2[2] = "！！！";
		talking2[3] = "哎呀哎呀，想不到你們還挺有兩下子";
		talking2[4] = "四大魔導．紅蓮的赫里厄斯......";
		talking2[5] = "呀，原來我這麼出名嗎？";
		talking2[6] = "嘛，總之請你把黑盒子交給我吧，不然我會很困擾的";
		talking2[7] = "......要是在這裡交給你的話，過去人們的努力不就白費了";
		talking2[8] = "哎呀...看來只能用強硬的方式讓你交來了";
		int word_count2[9] = {17,0,3,16,14,11,23,25,18};

		talk = 0;
		display = 0;
		shake = 0;
		shake_times = 0;
		direction = -1;
		cls_pos = 0;
		while (!quit && !skip)
		{
			fps.start();

			if (talk < 9)
			{
				display_end = false;
				display++;
				if (display > word_count2[talk] * 3)
				{
					display = word_count2[talk] * 3;
					display_end = true;
				}
			}
			while(SDL_PollEvent(&e))
			{
				if(e.type==SDL_QUIT) //單擊右上角的X
				{
					quit=true;
				}
				if(e.type==SDL_KEYDOWN)
				{
					switch(e.key.keysym.sym)
					{
						//ESC退出
						case SDLK_ESCAPE:
							quit=true;
							break;
						case SDLK_TAB:
							skip=true;
							break;
					}
				}
				if(e.type==SDL_MOUSEBUTTONUP && talk != 1)
				{
					if (!display_end)
					{
						display = word_count2[talk] * 3;
						display_end = true;
					}
					else
					{
						if (talk < 9)
							talk++;
						if (talk == 1)
							Mix_PlayChannel(-1,fire,0);
						else if (talk == 9 && cls_pos == 0)
						{
							Mix_PlayChannel(-1,transition,0);
							Mix_FadeOutMusic(600);
						}
						display = 0;
					}
				}
			}
			Window::clear();

			if (talk != 1)
			{
				bg4.draw(0,0);
				chat->draw(0,320);
			}
			else
			{
				shake += direction * 15;
				shake_times++;
				if (shake == -30)
					direction = 1;
				if (shake == 30)
					direction = -1;
				if (shake_times == 28)
				{
					talk = 2;
					bg4.draw(0,0);
					Window::present();
					SDL_Delay(3000);
					while (SDL_PollEvent(&trash_e));
					continue;
				}
				bg4.draw(shake,0);
				big_fire[shake_times/5]->draw(shake,0);
			}
			///////////////////////////////////////////表情
			if (talk == 0 || talk == 2)
				face1->draw(0,320);
			else if (talk == 4 || talk == 7)
				face1_2->draw(0,320);
			else if (talk != 1)
				face4->draw(0,320);
			///////////////////////////////////////////表情
			if (talk < 9)
			{
				if (display <= 54)
				{
					Texture* temp = new Texture(talking2[talk].substr(0,(display/3)*3),rgb(255, 255, 255),24);
					temp->setDstRect(0,0,temp->setPoint());
					temp->draw(460,400);
					delete temp;
				}
				else
				{
					Texture* temp = new Texture(talking2[talk].substr(0,54),rgb(255, 255, 255),24);
					temp->setDstRect(0,0,temp->setPoint());
					temp->draw(460,376);
					Texture* temp2 = new Texture(talking2[talk].substr(54,(display/3)*3-54),rgb(255, 255, 255),24);
					temp2->setDstRect(0,0,temp2->setPoint());
					temp2->draw(460,424);
					delete temp;
					delete temp2;
				}
			}
			else if (talk == 9)
			{
				cls_pos += 20;
				black_w->draw(-800+cls_pos,0);
				if (cls_pos == 820)
				{
					SDL_Delay(200);
					cls_pos = 0;
					break;
				}
			}

			if (fps.ticks()*FPS<1000)
				SDL_Delay((1000/FPS)- fps.ticks());
			Window::present();
		}
		Window::quit();
		delete chat;
		delete black_w;
		delete face1;
		delete face1_2;
		for (int i = 0; i < 6; i++)
			delete big_fire[i];
		delete big_fire;
		if (quit)
			exit(0);
		skip=false;
		//////////////////////////////////////////////////切換戰鬥畫面
		Window::initialize("Eschatology");

		black = new Texture("img/black.png");
		Texture boss("img/boss.png");
		character1 = new Texture("img/character1.png");
		Texture character4("img/character4.png");
		hp_ground = new Texture("img/hp_ground.png");
		hp_layer = new Texture("img/hp_layer.png");
		hp2_ground = new Texture("img/hp2_ground.png");
		hp2_layer = new Texture("img/hp2_layer.png");
		aluren = new Texture("img/aluren.png");
		aluren_click = new Texture("img/aluren_click.png");
		buttonFlame = new Texture("img/buttonFlame.png");
		buttonFlame_p = new Texture("img/buttonFlame_p.png");
		buttonIce = new Texture("img/buttonIce.png");
		buttonIce_p = new Texture("img/buttonIce_p.png");
		buttonLight = new Texture("img/buttonLight.png");
		buttonLight_p = new Texture("img/buttonLight_p.png");
		buttonDark = new Texture("img/buttonDark.png");
		buttonDark_p = new Texture("img/buttonDark_p.png");
		battle_bg = new Texture("img/battle_bg.png");
		temp_circle = new Texture("img/circle.png");
		flame = new Texture* [6];
		ice = new Texture* [6];
		light = new Texture* [6];
		dark = new Texture* [6];
		spear = new Texture* [6];
		slash = new Texture* [6];
		bad_click = new Texture* [6];
		for (int i = 0; i < 6; i++)
		{
			string temp_name;
			temp_name = "img/anime/flame"+anime_frame[i];
			flame[i] = new Texture(temp_name);
			temp_name = "img/anime/ice"+anime_frame[i];
			ice[i] = new Texture(temp_name);
			temp_name = "img/anime/light"+anime_frame[i];
			light[i] = new Texture(temp_name);
			temp_name = "img/anime/dark"+anime_frame[i];
			dark[i] = new Texture(temp_name);
			temp_name = "img/anime/spear"+anime_frame[i];
			spear[i] = new Texture(temp_name);
			temp_name = "img/anime/flame"+anime_frame[i];
			slash[i] = new Texture(temp_name);
			temp_name = "img/anime/bad_click"+anime_frame[i];
			bad_click[i] = new Texture(temp_name);
		}
		transition_r = Mix_LoadWAV("sounds/transition_r.wav");
		flame_s = Mix_LoadWAV("sounds/flame.wav");
		ice_s = Mix_LoadWAV("sounds/ice.wav");
		light_s = Mix_LoadWAV("sounds/light.wav");
		dark_s = Mix_LoadWAV("sounds/dark.wav");
		spear_s = Mix_LoadWAV("sounds/spear.wav");
		slash_s = Mix_LoadWAV("sounds/flame.wav");
		mainBGM = Mix_LoadMUS("sounds/battle2.ogg");
		SDL_Delay(500);
		Mix_PlayChannel(-1,transition_r,0);

		display = 0;
		cls_pos = 0;
		hp1 = 100;
		int hp2_base;
		if(level==1)
			hp2_base = 1200;
		if(level==2 || level==3)
			hp2_base = 4000;
		hp2=hp2_base;

		skill = 0;
		click_effect = 0;
		in_battle = false;
		string levelWord[3]={"E","H","X"};
		ifstream song2("charts/battle2/battle2-"+levelWord[level-1]+".jc",ifstream::in); //依照一開始按的數字鍵決定難度
		chart = new ChartPlayer(song2);
		last_judge = 0;
		e_damage = 2;
		resist = 0.6;
		replay = false;
		int cls_pos2 = 0;

		fps.start();

		while (!quit && !skip)
		{
			if (in_battle)
			{
				judge = chart->getJudgements();
				for (int i = last_judge; i < judge.size(); i++)
				{
					if (judge[i] == MISS)
					{
						slash_anime.push_back(0);
						Mix_PlayChannel(-1,slash_s,0);
						hp1 -= e_damage;
						skill = 0;
					}
				}
				last_judge = judge.size();
			}

			if (attack.ticks() > 4000 && in_battle)
			{
				slash_anime.push_back(0);
				Mix_PlayChannel(-1,slash_s,0);
				hp1 -= e_damage;
				attack.restart();
			}

			while(SDL_PollEvent(&e))
			{
				if(e.type==SDL_QUIT) //單擊右上角的X
				{
					quit=true;
				}
				if(e.type==SDL_KEYDOWN)
				{
					if (e.key.keysym.sym == SDLK_ESCAPE)
					{
						//ESC退出
						quit=true;
						break;
					}
					else if(e.key.keysym.sym == SDLK_TAB)
					{
						skip=true;
						break;
					}
					else if (e.key.keysym.sym == SDLK_f || e.key.keysym.sym == SDLK_e || e.key.keysym.sym == SDLK_j || e.key.keysym.sym == SDLK_i)
					{
						if (in_battle)
						{
							switch (e.key.keysym.sym)
							{
								case SDLK_e:
									pressing[0] = true;
									break;
								case SDLK_f:
									pressing[1] = true;
									break;
								case SDLK_i:
									pressing[2] = true;
									break;
								case SDLK_j:
									pressing[3] = true;
									break;
							}
							chart->hit();
							judge = chart->getJudgements();
							for (int i = last_judge; i < judge.size(); i++)
							{
								if (judge[i] == MISS)
								{
									slash_anime.push_back(0);
									Mix_PlayChannel(-1,slash_s,0);
									hp1 -= e_damage;
									skill = 0;
								}
								else if (judge[i] != WRONG)
								{
									click_effect = 1;
									aluren_click->setColor(accuracy[judge[i]]);
									switch (e.key.keysym.sym)
									{
										case SDLK_e:
											Mix_PlayChannel(-1,flame_s,0);
											if (judge[i] == 4)
											{
												bad_click_anime.push_back(make_pair(0,rgb(255,0,0)));
												hp1 -= e_damage/2;
												skill = 0;
											}
											else
											{
												flame_anime.push_back(0);
												hp2 -= damage*damage_rate[judge[i]]*resist;
												if (skill == 0 || skill == 2)
													skill++;
												else
													skill = 0;
											}
											break;
										case SDLK_f:
											Mix_PlayChannel(-1,ice_s,0);
											if (judge[i] == 4)
											{
												bad_click_anime.push_back(make_pair(0,rgb(130,245,255)));
												hp1 -= e_damage/2;
												skill = 0;
											}
											else
											{
												ice_anime.push_back(0);
												hp2 -= damage*damage_rate[judge[i]]*resist;
												if (skill == 1)
													skill++;
												else
													skill = 0;
											}
											break;
										case SDLK_i:
											Mix_PlayChannel(-1,light_s,0);
											if (judge[i] == 4)
											{
												bad_click_anime.push_back(make_pair(0,rgb(255,255,130)));
												hp1 -= e_damage/2;
												skill = 0;
											}
											else
											{
												light_anime.push_back(0);
												hp2 -= damage*damage_rate[judge[i]]*resist;
												if (skill == 3)
													skill++;
												else
													skill = 0;
											}
											break;
										case SDLK_j:
											if (judge[i] == 4)
											{
												Mix_PlayChannel(-1,dark_s,0);
												bad_click_anime.push_back(make_pair(0,rgb(70,0,115)));
												hp1 -= e_damage/2;
											}
											else
											{
												hp2 -= damage*damage_rate[judge[i]]*resist;
												if (skill == 4)
												{
													hp2 -= 20;
													spear_anime.push_back(0);
													Mix_PlayChannel(-1,spear_s,0);
												}
												else
												{
													Mix_PlayChannel(-1,dark_s,0);
													dark_anime.push_back(0);
												}
											}
											skill = 0;
											break;
									}
								}
							}
							last_judge = judge.size();
						}
					}
				}
				if(e.type==SDL_KEYUP)
				{
					switch (e.key.keysym.sym)
					{
						case SDLK_e:
							pressing[0] = false;
							break;
						case SDLK_f:
							pressing[1] = false;
							break;
						case SDLK_i:
							pressing[2] = false;
							break;
						case SDLK_j:
							pressing[3] = false;
							break;
					}
				}
			}
			if (fps.ticks() > 16)
			{
				fps.restart();
				Window::clear();
				boss.draw(0,0);
				character1->draw(0,0);
				character4.draw(280,0);
				battle_bg->draw(0,180);
				if (pressing[0])
					buttonFlame_p->draw(16,589);
				else
					buttonFlame->draw(16,589);
				if (pressing[1])
					buttonIce_p->draw(98,678);
				else
					buttonIce->draw(98,678);
				if (pressing[2])
					buttonLight_p->draw(374,589);
				else
					buttonLight->draw(374,589);
				if (pressing[3])
					buttonDark_p->draw(292,678);
				else
					buttonDark->draw(292,678);

				if (in_battle)
				{
					hp_ground->draw(0,180);
					hp2_ground->draw(240,180);
					hp_layer->draw(hp1*240/100-240,180);
					hp2_layer->draw(480-hp2*240/hp2_base,180);
					stringstream ss;
					stringstream ss2;
					string temp_hp;
					ss << hp1;
					ss >> temp_hp;
					Texture* hp1_t = new Texture(temp_hp+"/100",rgb(255, 255, 255),16);
					ss2 << hp2;
					ss2 >> temp_hp;
					Texture* hp2_t = new Texture(temp_hp+"/"+to_string(hp2_base),rgb(255, 255, 255),16);
					hp1_t->draw(15,195);
					hp2_t->draw(465-hp2_t->getWidth(),195);
					aluren->draw(90,270);
				}
				if (click_effect > 0)
				{
					aluren_click->setAlpha(255-17*click_effect);
					aluren_click->draw(80,260);
					click_effect++;
					if (click_effect == 15)
						click_effect = 0;
				}
				////////////////////////////////////////////////////////譜面顯示
				for (int t = 0; t < 1 && in_battle; t++)
				{
					auto temp_chart = chart->getNotePositions(BEAT_DURATION);
					for (int i = 0; i < temp_chart.size(); i++)
					{
						int temp_r = 300*(BEAT_DURATION-temp_chart.at(i).first)/BEAT_DURATION;
						if (temp_r == 0)
							temp_r = 2;
						temp_circle->setDstRect(temp_r,temp_r);
						temp_circle->draw(240-temp_r/2,420-temp_r/2);
					}
				}
				////////////////////////////////////////////////////////動畫表現
				for (int i = 0; i < flame_anime.size(); i++)
				{
					flame[flame_anime.at(i)/2]->draw(280,0);
					flame_anime.at(i)++;
					if (flame_anime.at(i) == 12)
						flame_anime.erase(flame_anime.begin()+i);
				}
				for (int i = 0; i < ice_anime.size(); i++)
				{
					ice[ice_anime.at(i)/2]->draw(280,0);
					ice_anime.at(i)++;
					if (ice_anime.at(i) == 12)
						ice_anime.erase(ice_anime.begin()+i);
				}
				for (int i = 0; i < light_anime.size(); i++)
				{
					light[light_anime.at(i)/2]->draw(280,0);
					light_anime.at(i)++;
					if (light_anime.at(i) == 12)
						light_anime.erase(light_anime.begin()+i);
				}
				for (int i = 0; i < dark_anime.size(); i++)
				{
					dark[dark_anime.at(i)/2]->draw(280,0);
					dark_anime.at(i)++;
					if (dark_anime.at(i) == 12)
						dark_anime.erase(dark_anime.begin()+i);
				}
				for (int i = 0; i < spear_anime.size(); i++)
				{
					spear[spear_anime.at(i)/2]->draw(280,0);
					spear_anime.at(i)++;
					if (spear_anime.at(i) == 12)
						spear_anime.erase(spear_anime.begin()+i);
				}
				for (int i = 0; i < slash_anime.size(); i++)
				{
					slash[slash_anime.at(i)/2]->draw(0,0);
					slash_anime.at(i)++;
					if (slash_anime.at(i) == 12)
						slash_anime.erase(slash_anime.begin()+i);
				}
				for (int i = 0; i < bad_click_anime.size(); i++)
				{
					bad_click[bad_click_anime.at(i).first/2]->setColor(bad_click_anime.at(i).second);
					bad_click[bad_click_anime.at(i).first/2]->draw(0,0);
					bad_click_anime.at(i).first++;
					if (bad_click_anime.at(i).first == 12)
						bad_click_anime.erase(bad_click_anime.begin()+i);
				}
				/////////////////////////////////////////////////////////////////
				if (cls_pos < 800)
				{
					black->draw(0,cls_pos);
					cls_pos += 20;
					display += 6;
					hp_layer->draw(-240+display,180);
					hp2_layer->draw(480-display,180);
					Texture* hp1_t = new Texture("100/100",rgb(255, 255, 255),16);
					Texture* hp2_t = new Texture(to_string(hp2_base)+"/"+to_string(hp2_base),rgb(255, 255, 255),16);
					hp1_t->setAlpha(display+15);
					hp1_t->draw(15,195);
					hp2_t->setAlpha(display+15);
					hp2_t->draw(465-hp2_t->getWidth(),195);
					aluren->setAlpha(display+15);
					aluren->draw(90,270);
					delete hp1_t;
					delete hp2_t;
					if (cls_pos == 800)
					{
						in_battle = true;
						Mix_PlayMusic(mainBGM,0);
						Mix_HookMusicFinished(replayBGM);
						chart->start();
						attack.start();
					}
				}
				if (hp1 <= 0)
				{
					in_battle = false;
					black->setAlpha(cls_pos2);
					black->draw(0,0);
					cls_pos2 += 15;
					if (cls_pos2 == 15)
						Mix_FadeOutMusic(500);
					else if (cls_pos2 == 270)
					{
						quit = true;
						break;
					}
				}
				if (replay)
				{
					in_battle = false;
					attack.stop();
					black->setAlpha(cls_pos2);
					black->draw(0,0);
					cls_pos2 += 15;
					if (cls_pos2 == 15)
						Mix_FadeOutMusic(500);
					else if (cls_pos2 == 270)
					{
						replay = false;
						SDL_Delay(500);
						break;
					}
				}

				Window::present();
			}
		}
		Window::quit();
		song2.close();
		for (int i = 0; i < 6; i++)
		{
			delete flame[i];
			delete ice[i];
			delete light[i];
			delete dark[i];
			delete spear[i];
			delete slash[i];
			delete bad_click[i];
		}
		delete [] flame;
		delete [] ice;
		delete [] light;
		delete [] dark;
		delete [] spear;
		delete [] slash;
		delete [] bad_click;
		delete black;
		delete battle_bg;
		delete aluren;
		delete aluren_click;
		delete temp_circle;
		delete buttonFlame;
		delete buttonFlame_p;
		delete buttonIce;
		delete buttonIce_p;
		delete buttonLight;
		delete buttonLight_p;
		delete buttonDark;
		delete buttonDark_p;
		delete character1;
		delete hp_ground;
		delete hp_layer;
		delete hp2_ground;
		delete hp2_layer;
		delete chart;
		delete face4;
		if (quit)
			exit(0);
		skip=false;
		///////////////////////////////////////////////////////////////切回動畫畫面
		Window::initialize_wide("Eschatology");

		Texture bg5("img/bg5.png");
		chat = new Texture("img/chat.png");
		black_w = new Texture("img/black_w.png");
		Texture white("img/white.png");
		big_fire = new Texture* [6];
		for (int i = 0; i < 6; i++)
		{
			string temp_name;
			temp_name = "img/anime/fire"+anime_frame[i];
			big_fire[i] = new Texture(temp_name);
		}
		face1_2 = new Texture("img/face1_2.png");
		Texture face1_3("img/face1_3.png");
		face4 = new Texture("img/face4.png");
		Texture face5("img/face5.png");
		Texture end_bg("img/end_bg.png");
		Texture** flash = new Texture* [3];
		flash[0] = new Texture("img/flash1.png");
		flash[1] = new Texture("img/flash2.png");
		flash[2] = new Texture("img/flash3.png");

		mainBGM = Mix_LoadMUS("sounds/last_bgm.ogg");
		fire = Mix_LoadWAV("sounds/fire.wav");
		light_s = Mix_LoadWAV("sounds/light.wav");
		Mix_Chunk *last = Mix_LoadWAV("sounds/last.wav");

		Mix_PlayMusic(mainBGM,-1);

		talking2[0] = "呃！";
		talking2[1] = "不...行......";
		talking2[2] = "（倒）";
		talking2[3] = "嘛嘛，堅持到現在也真是辛苦你了";
		talking2[4] = "黑盒子我就拿走......";
		talking2[5] = "";
		talking2[6] = "哎呀哎呀，來了個不簡單的人物呢";
		talking2[7] = "反叛軍幹部，聖靈之光．米亞";
		talking2[8] = "不會讓你們得逞的";
		int word_count3[9] = {2,5,3,15,9,0,15,13,8};

		talk = -1;
		display = 0;
		shake_times = 0;
		cls_pos = 0;
		cls_pos2 = 0;
		while (!quit && !skip)
		{
			fps.start();

			if (talk < 9)
			{
				display_end = false;
				display++;
				if (display > word_count3[talk] * 3)
				{
					display = word_count3[talk] * 3;
					display_end = true;
				}
			}
			while(SDL_PollEvent(&e))
			{
				if(e.type==SDL_QUIT) //單擊右上角的X
				{
					quit=true;
				}
				if(e.type==SDL_KEYDOWN)
				{
					switch(e.key.keysym.sym)
					{
						//ESC退出
						case SDLK_ESCAPE:
							quit=true;
							break;
						case SDLK_TAB:
							skip=true;
							break;
					}
				}
				if(e.type==SDL_MOUSEBUTTONUP && talk != 5 && talk != -1)
				{
					if (!display_end)
					{
						display = word_count3[talk] * 3;
						display_end = true;
					}
					else
					{
						if (talk < 9)
							talk++;
						if (talk == 5)
							Mix_PlayChannel(-1,light_s,0);
						else if (talk == 9 && cls_pos2 == 0)
							Mix_PlayChannel(-1,last,0);
						display = 0;
					}
				}
			}
			Window::clear();

			if (talk == -1)
			{
				bg5.draw(0,0);
				cls_pos++;
				if (cls_pos == 1)
					Mix_PlayChannel(-1,fire,0);
				if (cls_pos == 30)
				{
					talk = 0;
					Window::present();
					SDL_Delay(2000);
					while (SDL_PollEvent(&trash_e));
					continue;
				}
				big_fire[cls_pos/5]->draw(0,0);
			}
			else
			{
				if (talk != 5)
				{
					bg5.draw(0,0);
					if (talk != 9 && talk != 10)
						chat->draw(0,320);
				}
				else
				{
					shake_times++;
					if (shake_times == 12)
					{
						talk = 6;
						bg5.draw(0,0);
						Window::present();
						SDL_Delay(2000);
						while (SDL_PollEvent(&trash_e));
						continue;
					}
					flash[shake_times/4]->draw(shake,0);
				}
				///////////////////////////////////////////表情
				if (talk == 0 || talk == 1)
					face1_2->draw(0,320);
				else if (talk == 2)
					face1_3.draw(0,320);
				else if (talk == 3 || talk == 4 || talk == 6 || talk == 7)
					face4->draw(0,320);
				else if (talk == 8)
					face5.draw(0,320);
				///////////////////////////////////////////表情
				if (talk < 9)
				{
					Texture* temp = new Texture(talking2[talk].substr(0,(display/3)*3),rgb(255, 255, 255),24);
					temp->setDstRect(0,0,temp->setPoint());
					temp->draw(460,400);
					delete temp;
				}
				else if (talk == 9)
				{
					cls_pos2 += 1;
					white.setAlpha(cls_pos2+15);
					white.draw(0,0);
					if (cls_pos2 == 240)
					{
						SDL_Delay(5000);
						cls_pos2 = 0;
						talk = 10;
						continue;
					}
				}
				if (talk == 10)
				{
					cls_pos2 += 2;
					end_bg.draw(0,0);
					if (cls_pos2 < 240)
					{
						white.setAlpha(240-cls_pos2);
						white.draw(0,0);
					}
					if (cls_pos2 >= 600)
					{
						if (cls_pos2 == 600)
							Mix_FadeOutMusic(2000);
						black_w->setAlpha(cls_pos2-585);
						black_w->draw(0,0);
						if (cls_pos2 == 840)
							break;
					}
				}
			}

			if (fps.ticks()*FPS<1000)
				SDL_Delay((1000/FPS)- fps.ticks());
			Window::present();
		}
		Window::quit();
		delete chat;
		delete black_w;
		delete face1_2;
		delete face4;
		for (int i = 0; i < 6; i++)
			delete big_fire[i];
		delete big_fire;
		for (int i = 0; i < 3; i++)
			delete flash[i];
		delete flash;
		if (quit)
			exit(0);
		skip=false;
		finished=true;
	}
}
