/*

  g++ -g test20190419F-0.cpp route99.cpp bus20190419.cpp person20190419.cpp db20190419.cpp -o test20190419F-0 -llibpq -lwsock32 -lws2_32 -static-libstdc++ -I"D:\PostgreSQL\10\include" -L"D:\PostgreSQL\10\lib" 


  g++ -g test20190419F-0.cpp route99.cpp bus20190419.cpp person20190419.cpp -o test20190419F-0 -lwsock32 -lws2_32

  g++ -c test20190419F-0.cpp

  ====================================================
  乗客1人、バス1台での強化学習プログラムを完成させるぞ
  ====================================================




  Fプログラム (強化学習モード)

  ■ 行動は結構簡単、↑↓→←停止の5状態の行動しかない
  ■ 問題は環境評価 3要素
     (1)自分以外のバスの場所
     (2)まだ自宅にいる乗客の場所
     (3)現在乗っている乗客の到着場所

  ■ ルートプログラムを9x9のroute99.cppに変更

 
  Bプログラム(始端・終端付きの完全オンデマンド)をベースとして、「始端・終端なしの完全オンデマンド」のプログラムを作成する。→   つまりdirectionの概念を消す
  (ベースは、"test20190311B-0.cpp")

  以後、"Eプログラム"と称呼する。
  「乗せるべき乗客が存在しない時は、バスを停止させる」
*/

//#include <unistd.h> // sleep(1)
#include <time.h> // nanosleep()
#include <stdio.h>
#include <stdlib.h>

#include <math.h> // atan2
#include <fcntl.h>
#include <string.h>
#include <winsock2.h>

#include <iostream>
#include <list>   // list 利用のため
using namespace std;

#include "test20190419.h"

STATE *p_first_state, *p_last_state;  

extern list<PERSON> person_list; // シミュレータ上に存在する全ての人間
extern list<BUS> bus_list; // シミュレータ上に存在する全てのバス

LOCATION bus_stop[10][10];
int global_clock; // main の中のint iと同じ値にする

// 引数にするのが面倒なのでグローバルに出しておく
int sd;
struct sockaddr_in addr;
WSADATA wsaData;

double d_walk_only[100][100];
double d_bus_only[100][100];
double d_walk_and_bus[100][100];

list<int> path_walk_only[100][100];   // 歩きだけの最小時間ルート
list<int> path_bus_only[100][100];   // 歩きだけの最小時間ルート
list<int> path_walk_and_bus[100][100];   // バスを含めた最小時間ルート

int total_home_time = 0;
int total_travel_time = 0;
int total_life_time = 0;

extern double d[100][100];  // d[i][k]：ノードiからノードkへの移動時間 
extern int via[100][100];  //  d[i][k]の間にある(少くとも1つの)中継ノード

//// d[100][100]の退避変数


extern int make_route_init(void); // route.cpp
extern int make_route_change_walk_only(void); // route.cpp
extern int make_route_change_walk_and_bus(void); // route.cpp
extern int make_route_change_bus_only(void); // route.cpp
extern int make_route_calc(void); // route.cpp
extern void printPath1(int start, int goal, list<int> *p );

extern int open_db();
extern int delete_db();
extern int read_db();
extern int write_db(int number, int action_type, int q);
extern int close_db();

#define rad2deg(a) ((a)/M_PI * 180.0) /* rad を deg に換算するマクロ関数 */
#define deg2rad(a) ((a)/180.0 * M_PI) /* deg を rad に換算するマクロ関数 */

double distance_km(double px1, double py1, double px2, double py2, double *rad_up, int *area, int *dis)
{
  /*
	メインルーチンの記述例
	double rad_up1;
	int area;
	d_km = distance_km(px1, py1, px2, py2, &rad_up1, &area); 
  */

  double earth_r = 6378.137;

  double loRe = deg2rad(px2 - px1); // 東西
  double laRe = deg2rad(py2 - py1); // 南北

  double EWD = cos(deg2rad(py1))*earth_r*loRe; // 東西距離
  double NSD = earth_r*laRe; //南北距離

  double distance = sqrt(pow(NSD,2)+pow(EWD,2));  

  // y/xの逆正接(アークタンジェント)を-π/2～π/2の範囲で返します。
  *rad_up = atan2(NSD, EWD);
  
  double pi = 3.141592654;

  if ((*rad_up >= -1.0 * pi / 4.0 ) && (*rad_up < pi / 4.0 )){
	*area = 0; // 315 ～ 45度
  }
  else if ((*rad_up >= pi / 4.0 ) && (*rad_up < pi / 4.0 * 3)){
	*area = 1; // 45 ～ 135度
  }
  else if ((*rad_up <= -1.0 * pi / 4.0 ) &&  (*rad_up > -1.0 * pi / 4.0 * 3)){
	*area = 3; // 225 ～ 315度
  } 
  else {
	*area = 2; // 135～225度 
  }

  // "3"の意味→ Waiting Person | Riden Person | Bus. 
  
  if (distance < 0.360){
	*dis = 3;
  }
  else if (distance < 0.720){
	*dis = 2;
  }
  else {
	*dis = 1;
  }

  return distance;
}



//class PERSON; // PERSONの定義はBUSの下にある



int make_bus_stop(void)
{
  // for (int y = -2; y < 3; y++){ // 0～4 → これを0～9にするということは、5を足せばいい(はず)
  for (int y = -2; y < 3 + 5; y++){ // 0～4 + 5
	//for (int x = -4; x < 5; x++){ // 0～8  → これを0～9にするということは、1を足せばいい
	for (int x = -4; x < 5 + 1; x++){ // 0～8 + 1
	  double px = center_x + diff_x * (double)x;
	  double py = center_y + diff_y * (double)y;
	  
	  bus_stop[x + 4][y + 2].px = px;
	  bus_stop[x + 4][y + 2].py = py;

	  //int id = ((x + 5) * 10 + (y + 3)) * (-1);

	}
  }
  return 0;
}	  

int open_udp(void)
{
  // winsock2の初期化
  WSAStartup(MAKEWORD(2,0), &wsaData);
  
  //socketシステムコール
  //ディスクリプタsdを取得する
  //SOCK_DGRAM <- UDP, SOCK_STREAM<- TCP
  if((sd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
	perror("socket");
	printf("error:%d\n",WSAGetLastError());
	return -1;
  }
  
  // 送信先アドレスとポート番号を設定する
  memset(&addr,0,sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(2000);   // ポート番号(htons)でネットワークバイトオーダーに変換
  addr.sin_addr.s_addr = inet_addr("127.0.0.1");  //IPアドレスを格納したin_addr構造体
  //addr.sin_addr.s_addr = inet_addr("49.212.198.156");  //IPアドレスを格納したin_addr構造体

  return 0;
}
  

int clean_qgis_display(void)
{
  char ch[64] ={0};

  for (int i = 0; i < 9; i++){
	sprintf(ch, "bus%d, 0.0, 0.0\n", i);
	if (sendto(sd, ch, strlen(ch), 0, (struct sockaddr *)&addr,  sizeof(addr)) < 0) {        
	  perror("sendto");
	  printf("error:%d\n",WSAGetLastError());
	  return -1;
	}
  }

  for (int i = 0; i < 100; i++){
	sprintf(ch, "person%02d, 0.0, 0.0\n", i);
	if (sendto(sd, ch, strlen(ch), 0, (struct sockaddr *)&addr,  sizeof(addr)) < 0) {        
	  perror("sendto");
	  printf("error:%d\n",WSAGetLastError());
	  return -1;
	}
  }
  return 0;
}


int close_udp(void)
{
  // 接続を切断する
  closesocket(sd);
  WSACleanup();

  return 0;
}

void init_state_list(STATE **p_first_state, STATE **p_last_state)
{
  STATE *p_top_state = (STATE *)malloc(sizeof(STATE));
  if(p_top_state == NULL) {
	printf("メモリが確保できません\n");
	exit(EXIT_FAILURE);
  }
  memset(p_top_state, 0, sizeof(STATE)); // ゼロクリア


  STATE *p_tail_state = (STATE *)malloc(sizeof(STATE));
  if(p_tail_state == NULL) {
	printf("メモリが確保できません\n");
	exit(EXIT_FAILURE);
  }
  memset(p_tail_state, 0, sizeof(STATE)); // ゼロクリア

  *p_first_state = p_top_state;
  *p_last_state = p_tail_state;

  (*p_first_state)->prev = NULL;
  (*p_last_state)->next = NULL;
  (*p_first_state)->next = (*p_last_state);
  (*p_last_state)->prev = (*p_first_state);

  return;
}


STATE* make_state(int num, int action, int q)
{
  STATE *new_p_state = (STATE *)malloc(sizeof(STATE));
  if(new_p_state == NULL) {
	printf("メモリが確保できません\n");
	exit(EXIT_FAILURE);
  }
  memset(new_p_state, 0, sizeof(STATE)); // ゼロクリア

  return new_p_state;
}


STATE* add_state(STATE *p_ref_state)
{
  /*
	add_の使い方の注意

	  STATE state; // ローカルで実体を作っておいて

	  number = 123456;
	  action = rnd[1]; 
	  value = 4;
	  
	  add_state(&state); // add_の中でmallocする
  */

  STATE *new_p_state = (STATE *)malloc(sizeof(STATE));
  if(new_p_state == NULL) {
	printf("メモリが確保できません\n");
	exit(EXIT_FAILURE);
  }
  memset(new_p_state, 0, sizeof(STATE)); // ゼロクリア
  memcpy(new_p_state, p_ref_state, sizeof(STATE)); // 引数の動的メモリの内容コピー
 
  write_db(new_p_state->number, new_p_state->action_type, new_p_state->q);
	

  // stateの追加属性記述ここから
 
  // stateの追加属性記述ここまで

  STATE *p_state = p_last_state->prev;

  p_state->next = new_p_state;
  new_p_state->prev = p_state;

  p_last_state->prev = new_p_state;
  new_p_state->next = p_last_state;

  return new_p_state;
}

int change_or_add_state(int num, int action_type, int add_q) // 状態番号を入れて、stateの値を変更す
{
  // ===== STATE ループを回す========
  STATE* p_state = p_first_state->next;  
    while (p_state != p_last_state){  
	  /// 処理内容(開始)

	  //「状態」番号と、「行動」番号が一致する場合に
	  if ((p_state->number == num) && (p_state->action_type == action_type)){ 

		// Q値を増やす
		p_state->q += add_q;

		// 目的を達したら、とっとと出る
		return 1;  // この場合、返り値1 
	  }

	  /// 処理内容(終了)

	  // 次のループに回る
	  p_state = p_state->next;
	}

	/// 状態numは発見できなかったので、新しい「状態」を作る

	STATE state; // ローカルで中身を作っておいて
	  
	state.number = num;
	state.action_type = action_type;
	state.q = add_q; // 加算処理はしない
	
	add_state(&state); // add_の中でmalloc(実体化する)

	return 2; // この場合、返り値2

}


void delete_state(STATE *p_state)  
{
  /* 
	 delete_ の使い方の注意

	 STATE* p_state_prev = p_state->prev;
	 delete_state(p_state);
	 p_state = p_state_prev;
  */
	 

  // ポインタを貼り替えて
  p_state->prev->next = p_state->next;
  p_state->next->prev = p_state->prev;
  
  // そのメモリを解放する
  free(p_state);

  return;

}

void show_state()
{

  int number = 0;
  printf("start show_state()\n");

  STATE* p_state = p_first_state;
  while (p_state != p_last_state){

	printf("=========\n");
	printf("number:%d\n", p_state->number);
	printf("action_type:%d\n", p_state->action_type);
	printf("q:%d\n", p_state->q);

	write_db(p_state->number, p_state->action_type, p_state->q);
	
	number += 1;
	p_state = p_state->next;

  }
  printf("end of  show_state() total number:%d\n", number);

}


  

  

// 4進数表現とみなして、「状態」番号を10進数で表示
// メインルーチンでの使い方
/*
  int f[3][4] = {{1,0,1,0},{1,0,1,0},{1,0,1,0}};
  int a = make_number((int *)f);
*/

int make_number(int* field_check)
{
  int cc = 0;

  for (int i = 0; i < 3; i++){
	for (int k = 0; k < 4; k++){
	  cc *= 4;
	  cc = cc + *(field_check + i * 4 + k);
	}
  }

  return cc;
}


/*
  作らなければならないものは、バスの行動計画 と 人間の行動計画
  まず考え方を整理する

  バスは定常運行で

  3.6秒x25ステップ = 90秒で移動する。 さらに、3.6秒 x 5 = 18秒停車する
  1路線片道通常運転で 90x8 + 18x 7 = 846秒 = 14.1分の運行をする
  終端で180秒  秒停車して、再び運行開始 する
  ―― とする。

  次に乗客である。
  人間の歩行は時速3.6kmとする (秒速1m)

  最初は運行開始前に10人程作成して、その往路と帰路を全部網羅するルートを走るものとする

  今日の目標はここまで
*/ 

int make_routes()
{
  ////////// ルートの事前作成(歩行だけのルート)
  make_route_init();
  make_route_change_walk_only(); // route.cpp
  make_route_calc();
  // list<int> path_walk_only[100][100];   // 面倒なので、グローバルに持っていく
  memcpy(d_walk_only, d, sizeof(d));

  //printf("pass2\n");

  //printf("\n[TRY1]\n");
  for(int i = 0; i < 100; i++){
	for(int k = 0; k < 100; k++){
	  if ((d[i][k] < 999.0) && (d[i][k] > 0.000001)) {
		//printf("d[%02d][%02d]:%f ",i,k,d[i][k]);
		//printPath1(i, k);
		printPath1(i, k, &(path_walk_only[i][k]));
	  }
	}
  }

  ////////// ルートの事前作成(バスだけのルート)
  make_route_init();
  make_route_change_bus_only(); // route.cpp
  make_route_calc();
  // list<int> path_bus_only[100][100];   // 面倒なので、グローバルに持っていく
  memcpy(d_bus_only, d, sizeof(d));

  //printf("pass2\n");

  //printf("\n[TRY1]\n");
  for(int i = 0; i < 100; i++){
	for(int k = 0; k < 100; k++){
	  if ((d[i][k] < 999.0) && (d[i][k] > 0.000001)) {
		//printf("d[%02d][%02d]:%f ",i,k,d[i][k]);
		//printPath1(i, k);
		printPath1(i, k, &(path_bus_only[i][k]));
	  }
	}
  }

  ////////// ルートの事前作成(歩行とバスの混在ルート)
  make_route_init();
  make_route_change_walk_and_bus(); // 路線内容の強制上書き
  make_route_calc();
  
  // list<int> path_walk_and_bus[100][100];   // 面倒なので、グローバルに持っていく
  memcpy(d_walk_and_bus, d, sizeof(d));  

  //printf("\n[TRY2]\n");
  for(int i = 0; i < 100; i++){
	for(int k = 0; k < 100; k++){
	  if ((d[i][k] < 999.0) && (d[i][k] > 0.000001)) {
		//printf("d[%02d][%02d]:%f ",i,k,d[i][k]);
		//printPath1(i, k);
		printPath1(i, k, &(path_walk_and_bus[i][k]));
	  }
	}
  }
  return 0;
}


int person_list_flag = 0;

int speed_down_count = 0;

int main()
{
  init_state_list(&p_first_state, &p_last_state);

  open_db();

  //// q_stateテーブルをどうするかをここで決める


  // データベースの全削除(最初からやりなおす)
  // ↓
  delete_db(); 

  // データベースを削除をしないで読み込む(学習結果をQGISなどでで見たい時はこちら)
  // ↓
  //read_db();  
  


  //printf("pass1\n");

  make_routes();
  ////////// ルートの事前作成(ここまで)

  make_bus_stop(); // バス停の作成
  open_udp(); // 通信初期化
  
  // BUS bus1("bus1",0,2);
  // BUS *bus1 = new BUS("bus1",0,2,1);
  // BUS *bus1 = new BUS("bus1",8,2,2);
  
  // バスの生成
	/////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////
  for(int i = 1; i <= 1; i++){ // バスの台数はここで調整
	//////////////////////////////////////2///////////////////////
	/////////////////////////////////////////////////////////////
	if ( i == 1){
	  char name[10] ={0};
	  sprintf(name,"bus%d", i);
	  //BUS *bus = new BUS(name,0,2,1);
	  BUS *bus = new BUS(name,2,2,1);
	  bus->init_state_seq_list();
	  
	  // リストの追加
	  bus_list.push_back(*bus);

	}
	else if (i == 2){
	  char name[10] ={0};
	  sprintf(name,"bus%d", i);
	  BUS *bus = new BUS(name,8,2,2);
	  bus->init_state_seq_list();

	  // リストの追加
	  bus_list.push_back(*bus);
	}
	if ( i == 3){
	  char name[10] ={0};
	  sprintf(name,"bus%d", i);
	  BUS *bus = new BUS(name,4,2,3);
	  bus->init_state_seq_list();
	  
	  // リストの追加
	  bus_list.push_back(*bus);

	}
	if ( i == 4){
	  char name[10] ={0};
	  sprintf(name,"bus%d", i);
	  BUS *bus = new BUS(name,4,2,4);
	  bus->init_state_seq_list();
	  
	  // リストの追加
	  bus_list.push_back(*bus);
	}
  }

  // 状態リストの初期化(バス毎に作られる)
  list<BUS>::iterator b_pos;
  for(b_pos = bus_list.begin(); b_pos!=bus_list.end(); ++b_pos){	
	b_pos->init_state_seq_list();
  }


  // list<PERSON> person_list;
  
  srand(19);

  while(1){

	int person_num = 1;
  
	int finish_flag = 0;
	
	////////////////////////////
	// 計測用変数
	static int bus_status_count_0 = 0; // 0:停止中 
	static int bus_status_count_1 = 1; // 1:運行中 
	////////////////////////////
	
	////////////////////////////
	// 計測用変数
	static int person_status_count_0 = 0; // 0:出発前 
	static int person_status_count_1 = 0; // 1:停止中 
	static int person_status_count_2 = 0; // 2:歩行中
	static int person_status_count_3 = 0; // 3:乗車中
	static int person5_status_count_2 = 0; // 2:歩行中
	////////////////////////////

	clean_qgis_display(); // QGIS画面のクリア

	person_list_flag = 0;
	person_list.clear(); // person_listの削除

	int i = 0;
	
	/////////////////////////////////////////////////////////////
	//for(int i = 0; i < 10000; i++){
	for(int i = 0; i < 100000; i++){
	//while(1){
	  /////////////////////////////////////////////////////////////
	  
	  printf("%d \n",i);
	  global_clock = i; // グローバルクロックは、グローバル変数
	  
	  // 「なんでもサブルーチン」はやめよう
	  // メインルーチンで手を抜こう(どうせ再利用なんかしないしぃ)
	  
	  //	if (person_num < 10){ // 最大10人作る
	  /////////////////////////////////////////////////////////////
	  /////////////////////////////////////////////////////////////
	  
	  if (person_num <= 1){ // 9人作る
		//if (rand() % 25 == 0){
		if (rand() % 1 == 0){
		  
		  //if (person_num <= 100){ // 100人作る
		  //if (rand() % 100000 == 0){
		  
		  ////////////////////////////////////////////////////////////
		  /////////////////////////////////////////////////////////////
		  // 25ループに1回くらいの確率で利用者を発生させることにする
		  // 利用者の行き先もテキトーに決める
		  int dep_grid_x;
		  int dep_grid_y;
		  int arr_grid_x;
		  int arr_grid_y;
		  
		  do {
			//dep_grid_x = rand() % 9; // 0～8までの値が出てくる
			dep_grid_x = rand() % 10; // 0～9までの値が出てくる
			
			//dep_grid_y = rand() % 5; // 0～4までの値が出てくる
			dep_grid_y = rand() % 10; // 0～9までの値が出てくる
			
			// arr_grid_x = rand() % 9; // 0～8までの値が出てくる
			arr_grid_x = rand() % 10; // 0～9までの値が出てくる
			
			//arr_grid_y = rand() % 5; // 0～4までの値が出てくる
			arr_grid_y = rand() % 10; // 0～9までの値が出てくる
			
			// 2つのgridは同じ場所であってはならない(当然)
			//↓
			// 2つのgridの距離は、500メートル以上(実質750メートル以上離れていることとする
			//} while((dep_grid_x == arr_grid_x) && (dep_grid_y == arr_grid_y))
		  } while (( abs(dep_grid_x - arr_grid_x) + abs(dep_grid_y - arr_grid_y)) <= 3 );
		  
		  char name[10] ={0};
		  sprintf(name,"person%02d", person_num);
		  
		  cout << "i =" << i << "\n";
		  cout << "person_num =" << person_num << " " << name << "\n";
		  cout << "x:" << dep_grid_x << " y:" << dep_grid_y << " x:" 
			   << arr_grid_x << " y:" << arr_grid_y << "\n";
		  
		  // 乗客の生成
		  PERSON *person 
			= new PERSON(person_num, name, dep_grid_x, dep_grid_y, arr_grid_x, arr_grid_y);
		  
		  // person->send_udp();
		  
		  // リストへの追加
		  person_list.push_back(*person);
		  // delete person; // 削除していいのか？
		  
		  person_num += 1;
		}
	  }
	  
	  /////////////////////////////////////////////////////////////////////
	  

#if 1

	  if ((speed_down_count % 5 == 0) && (speed_down_count != 0)){
		  

		struct timespec ts;
		ts.tv_sec = 0;
		ts.tv_nsec = 999999999/999; // 1/350秒と見なして良い
		nanosleep(&ts, NULL);
		

	  }
		

#endif

	  //usleep(100000/35); // 0.2秒
	  //usleep(100000/5); // 0.2秒
	  
	  
	  //usleep(100000); // 0.2秒
	  //sleep(1);
	  /////////////////////////////////////////////////////////////////////
	  
	  list<BUS>::iterator b_pos;
	  for(b_pos = bus_list.begin(); b_pos!=bus_list.end(); ++b_pos){	
		
		//b_pos->driver();
		b_pos->driver_q();
		b_pos->send_udp();
	  }
	  
	  list<PERSON>::iterator pos;
	  for(pos = person_list.begin(); pos!=person_list.end(); ++pos){
		pos->walk_only_check(); 
		pos->send_udp();	  
	  } 
	  
	  for(b_pos = bus_list.begin(); b_pos!=bus_list.end(); ++b_pos){	
		for(pos = person_list.begin(); pos!=person_list.end(); ++pos){
		  pos->walk_and_bus_check(b_pos); 
		  pos->send_udp();
		} 
	  }
	  
#if 0
	  // 観測と記録を投入
	  for(b_pos = bus_list.begin(); b_pos!=bus_list.end(); ++b_pos){
		b_pos->check_environment();
		b_pos->add_state_seq();
	  }
	  
#endif 
  	  
	  // 計測専用ルーチン
	  for(b_pos = bus_list.begin(); b_pos!=bus_list.end(); ++b_pos){
		if (b_pos->bus_status == 0)
		  bus_status_count_0 += 1;
		else if (b_pos->bus_status == 1)
		  
		  bus_status_count_1 += 1;
	  }
	  
	  for(pos = person_list.begin(); pos!=person_list.end(); ++pos){
		if (pos->person_status == 0)
		  person_status_count_0 += 1;
		else if (pos->person_status == 1)
		  person_status_count_1 += 1;
		else if (pos->person_status == 2)
		  person_status_count_2 += 1;
		else if (pos->person_status == 3)
		  person_status_count_3 += 1;
	  }
	  
	  // 到着乗客をリストから外す(が、その前に必要な情報を取り出す)
	  pos = person_list.begin();
	  while( pos != person_list.end()) {
		if (pos->person_status == -1){
		  person_list_flag = 1; //フラグを上げる
		  
		  cout << "Erased person num:" << pos->number <<"\n";
		  cout << "home_time:" << pos->home_time <<"\n";
		  
		  if (pos->arr_time == -1){ // 到着時刻を記録
			pos->arr_time = global_clock;
			pos->travel_time = pos->arr_time - pos->dep_time;
		  }
		  cout << "travel_time:" << pos->travel_time <<"\n";
		  
		  if (pos->life_time == -1){ // 到着時刻を記録
			pos->life_time = pos->arr_time - pos->gen_time;
		  }
		  cout << "travel_time:" << pos->travel_time <<"\n";
		  
		  
		  total_home_time += pos->home_time;
		  total_travel_time += pos->travel_time;
		  total_life_time += pos->life_time;
		  
		  pos = person_list.erase(pos);
		}
		else 
		  pos++;
	  }
	  
	  // 終了判定
	  if ((person_list_flag == 1) && (person_list.size() == 0)){
		printf("bus_status_count_0 = %d\n",bus_status_count_0);
		printf("bus_status_count_1 = %d\n",bus_status_count_1);
		printf("person_status_count_0 = %d\n",person_status_count_0); // 一時停止
		printf("person_status_count_1 = %d\n",person_status_count_1); // 1:歩行時間
		printf("person_status_count_2 = %d\n",person_status_count_2); // 2:バス待ち時間
		printf("person_status_count_3 = %d\n",person_status_count_3); // 3:バス乗車時間
		
		printf("total_home_time = %d\n",total_home_time);
		printf("total_travel_time = %d\n",total_travel_time);
		printf("total_life_time = %d\n",total_life_time);
		
		
		for(b_pos = bus_list.begin(); b_pos!=bus_list.end(); ++b_pos){
		  if ((100000 - i) > 1){
			b_pos->q_reward(100000 - i);
			
			speed_down_count += 1;		
			printf("q_reward speed_down_count %d\n ", speed_down_count);

			//b_pos->q_reward(10*(10000 - i));
			b_pos->del_state_seq(); // seqリストを削除
		  }
		}
	
		show_state();
		person_list_flag = 0;		
		
		finish_flag = 1;

		//exit(0);
	  }	// 終了判定 if ((person_list_flag == 1) && (person_list.size() == 0))

	  if (finish_flag == 1){
		finish_flag = 0;
		break;
	  }
	}
	
	for(b_pos = bus_list.begin(); b_pos!=bus_list.end(); ++b_pos){
	  b_pos->del_state_seq(); // seqリストを削除
	}

  }  // while(1)

  close_udp();
  close_db();

}
