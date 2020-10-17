/*
  g++ -c bus20190419.cpp
*/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // sleep(1)
#include <math.h> // atan2
#include <fcntl.h>
#include <string.h>
#include <winsock2.h>

#include <iostream>
#include <list>   // list 利用のため
using namespace std;

#include "test20190419.h"

extern STATE *p_first_state, *p_last_state;  


extern double distance_km(double px1, double py1, double px2, double py2, double *rad_up, int *area, int *dis);

extern STATE* make_state(int num, int action, int q);
extern int make_number(int* field_check);
extern STATE* add_state(STATE *p_ref_state);

extern list<PERSON> person_list; // シミュレータ上に存在する全ての人間(mainにいます)
list<BUS> bus_list; // シミュレータ上に存在する全てのバス

extern int sd;
extern struct sockaddr_in addr;
extern WSADATA wsaData;

extern LOCATION bus_stop[10][10];
extern double d_bus_only[100][100];
extern double d_walk_only[100][100];
extern double d_walk_and_bus[100][100];

extern list<int> path_bus_only[100][100];   // 歩きだけの最小時間ルート


RND rnd[] = {{ 0, 0},  
			 { 1, 0},  
			 { 0, 1},  
			 {-1, 0},  
			 { 0,-1}}; 


BUS::BUS(const char *n, int prev_grid_gx, int prev_grid_gy, int route_case){

  number = route_case; // ケースをバスの番号にする("-1"の値はありえない)
  
  strcpy(name, n);
  
  stop_timer = 5; // バス停留所のステップ数
  bus_status = 1; // バス運行:1 バス停車:0
  
  prev_grid.gx = next_grid.gx = prev_grid_gx;
  prev_grid.gy = next_grid.gy = prev_grid_gy;
  
  present.px = prev_bus_stop.px = next_bus_stop.px 
	= bus_stop[prev_grid_gx][prev_grid_gy].px;
  present.py = prev_bus_stop.py = next_bus_stop.py 
	= bus_stop[prev_grid_gx][prev_grid_gy].py;
  
  ////// バスの往復ルートの作成
  list<int> p_bus_outward;
  list<int> p_bus_homeward;

#if 0  
  if (route_case == 1){
	direction = 1;
  }
  else if (route_case == 2){
	direction = 2;
  }
  else if (route_case == 3){
	direction = 1;
  } 
  else if (route_case == 4){
	direction = 2;
  } 
  else{
	printf("Error case %d\n", route_case);
	exit(1);
  }
#endif //0 
  // 「指向性なし」と定義する
  direction = -1;  
  
  
  // とまあ、十分大きな数を作っておけば、良いかな、と、
  
  ////// バスの往復ルートの作成(終わり)
  
#if 0	
  list<int>::iterator pos;
  for(pos = p_bus_only_path.begin(); pos!=p_bus_only_path.end(); ++pos){
	cout << *pos << "\n";
  }
#endif
  
  // exit(1);
  
}

int BUS::send_udp()
{
  char ch[64] ={0};
  
  //send_udp(bus1->name, bus1->present.px ,bus1->present.py);	
  sprintf(ch, "%s,%f,%f\n",name, present.py, present.px);
  
  //  printf("%s",ch); // 確認用
  
  if (sendto(sd, ch, strlen(ch), 0, (struct sockaddr *)&addr,  sizeof(addr)) < 0) {        
	perror("sendto");
	printf("error:%d\n",WSAGetLastError());
	return -1;
  }
  return 0;
}

int BUS::bus_change_direction(int next_grid_gx, int next_grid_gy){ // 次の経路を示す

  printf("1 ::bus_change_direction:prev.....%d (%d,%d)\n",number, prev_grid.gx, prev_grid.gy);  
  printf("1 ::bus_change_direction:next.....%d (%d,%d)\n",number, next_grid.gx, next_grid.gy);  


  // 領域制限
  if (next_grid_gx > 9) 
	next_grid_gx = 9;
  else if (next_grid_gx < 0) 
	next_grid_gx = 0;

  if (next_grid_gy > 9) 
	next_grid_gy = 9;
  else if (next_grid_gy < 0) 
	next_grid_gy = 0;
  // 領域制限(ここまで)
  
  prev_grid.gx = next_grid.gx;
  prev_grid.gy = next_grid.gy;
  
  next_grid.gx = next_grid_gx;
  next_grid.gy = next_grid_gy;
  
  prev_bus_stop.px = next_bus_stop.px;
  prev_bus_stop.py = next_bus_stop.py;	
  next_bus_stop.px = bus_stop[next_grid.gx][next_grid.gy].px;
  next_bus_stop.py = bus_stop[next_grid.gx][next_grid.gy].py;
  
  // 右端停留所(82)(に到着してるのであれば、下り(2)とする。
  // 左端停留所(02)(に到着してるのであれば、上り(1)とする。

#if 0  
  if ((prev_grid.gx == 8) && (prev_grid.gy ==2)){
	direction = 2;
  }
  else if ((prev_grid.gx == 0) && (prev_grid.gy ==2)){
	direction = 1;
  }
#endif //0
  
  printf("2 ::bus_change_direction:prev.....%d (%d,%d)\n",number, prev_grid.gx, prev_grid.gy);  
  printf("2 ::bus_change_direction:next.....%d (%d,%d)\n",number, next_grid.gx, next_grid.gy);  

  return 0;
}

  
int BUS::check_environment()
{

  list<BUS>::iterator b_pos;
  
  double rad_up1;
  int area;
  int dis;


  // まず状態変数をリセットする
  memset(field_check, 0, sizeof(field_check));

  // 自分の乗せている乗客の降車位置はどこだ？
  list<PERSON>::iterator bp;
  for(bp = passenger.begin(); bp!=passenger.end(); ++bp){	// バスに乗っている乗客
	if (bp->person_status == 3){ // 3:乗車中
	  // _arr_xx は意味が変っているが、変更すると面倒になるのでこのまま
	  
	  double d_km = distance_km(present.px, present.py, 
								bp->arrival.px, bp->arrival.py, 
								&rad_up1, &area, &dis); 

	  if (dis > field_check[0][area]){
		field_check[0][area] = dis;
	  }
	}
  }

  // バスを待っている乗客の位置はどこだ？ 

  list<PERSON>::iterator pos;
  for(pos = person_list.begin(); pos!=person_list.end(); ++pos){
	// バス乗車中(3)ではなく
	if (pos->person_status != 3){
	  // 到着(-1)もしていない
	  if (pos->person_status != -1){ 
		// 歩行中(0)でもない
		if (pos->person_status != 0){ 
		  
		  double d_km = distance_km(present.px, present.py, 
									pos->departure.px, pos->departure.py,
									&rad_up1, &area, &dis); 

		  if (dis > field_check[1][area]){
			field_check[1][area] = dis;

		  }
		}
	  }
	}
  }

  // 自分以外のバスの位置はどこだ？

  //list<BUS>::iterator b_pos;
  for(b_pos = bus_list.begin(); b_pos!=bus_list.end(); ++b_pos){	
	if (b_pos->number != number){ // 自分以外のバス
	  
	  double d_km = distance_km(present.px, present.py, 
								b_pos->present.px, b_pos->present.py, 
								&rad_up1, &area, &dis); 
	  
	  if (dis > field_check[2][area]){
		field_check[2][area] = dis;
	  }
	}
  }
  

  printf("field_check = {{%d, %d, %d, %d}, {%d, %d, %d, %d}, {%d, %d, %d, %d}}\n",
		 field_check[0][0],
		 field_check[0][1],
		 field_check[0][2],
		 field_check[0][3],
		 field_check[1][0],
		 field_check[1][1],
		 field_check[1][2],
		 field_check[1][3],
		 field_check[2][0],
		 field_check[2][1],
		 field_check[2][2],
		 field_check[2][3]);

  //// 状態番号の付与 状態を4進数に見たてて、これを10進数の数字に変換する
  //// 現在4^12=16777216 で 5アクション必要になるから、 83886080
  //// int型の範囲が、	               -2147483648 ～ 2147483647
  //// となるので、状態はintで表現可能となるハズ
  //// 表現は Q[status_number][action_number] で良いと思う

  printf("before make_number\n");
  int number = make_number((int *)field_check);
  printf("after make_number\n");

  return number;
}

int BUS::driver_q()
{
  if (bus_status == 1){ // バスが動いている
	
	// まず、1ステップ分(3.6秒)を進める
	// const double diff_x = 0.00276; // X方向に250メートル
	// const double diff_y = 0.00225; // Y方向に250メートル
	// 1ステップに10m 進む → 時速10km だから、上の値を"25"で割れば良い
	// → (10.0/250.0)を乗算する
	
	double rad = atan2((next_bus_stop.py - present.py), 
					   (next_bus_stop.px - present.px));
	// presentを更新する
	present.py += diff_y * (10.0/250.0) * sin(rad);
	present.px += diff_x * (10.0/250.0) * cos(rad);
	
	double rad_0 = atan2((next_bus_stop.py - prev_bus_stop.py),
						 (next_bus_stop.px - prev_bus_stop.px));
	
	double rad_1 = atan2((next_bus_stop.py - present.py), 
						 (next_bus_stop.px - present.px));

	// 停留所を越えた // 完全直行の場合に判断ができないので、若干の誤差を入れる
	if (fabs(rad_0 -rad_1) >= pi * 0.5 - 0.0001){ 
	
	  bus_status = 0; // バスを停車させる
	  
	  // バス停に位置を固定する
	  present.py = next_bus_stop.py;
	  present.px = next_bus_stop.px;
	}
  }

  else if (bus_status == 0){ // バスが停車している
	stop_timer -= 1; // ステップ時間を1つ減算
	
	if ( stop_timer < 0){ // ステップ時間を使い果した

	  //printf("\n bus:bus stop\n");
	  
	  bus_status = 1; // バスを動かす
	  stop_timer = 5; // ステップ数を戻す
	  
	  int num = check_environment(); // 環境をチェックする

	  // ここまでは、driver()と同じ

	  // 新しい状態が登場したら、アクション(5つ)分を全部作って待機する、という方法を取る

	  // 状態の中に、アクションを入れ子にするということも考えたが、それをやると、Q学習で
	  // 過去に遡る時、状態を記憶しなければならないというのが面倒なので、アクションごとに
	  // 状態を作ることにする
	  
	  // 状態空間が5倍になるのは辛いので、状態が発生した段階で状態を追加し続ける方法を取る
	  
	  // まず現在の環境が状態として存在するか否かを確認する
	  
	  // numberで状態を探す
	  STATE* p_state = p_first_state->next;  
	  STATE* q_state = NULL;
	  
	  int q_max = -1;
	  while (p_state != p_last_state){  
		/// 処理内容(開始)
		//「状態」番号が一致する場合に
		if (p_state->number == num){
		  q_state = p_state;
		  break;
		}
		p_state = p_state->next;
	  }
	
	  if (q_state == NULL){  // 状態番号は見つけられなかった
		for (int i = 0; i < 5; i ++){  // 新設する

		  STATE st;
		  st.number = num;
		  st.action_type = i; 
		  st.q = rand() % 10 + 1; // とりあえず、初期値は1～10ということで
		  //st.q = rand() % 100; // とりあえず、初期値は0～99ということで
		  
		  STATE* s = add_state(&st);
		  
		  if (i == 0){
			q_state = s;
		  }
		
		 
		}
	  }

	  // これで状態は全部存在しているハズである。(処理時間が無駄ではあるが)

	  if ((double)rand()/RAND_MAX < 0.3){  // ε:0.3以下
	  //if ((double)rand()/RAND_MAX < 0.6){  // ε:0.3以下
	  //if ((double)rand()/RAND_MAX < 0.9){  // ε:0.3以下
		int d = rand() % 5;

		printf("\n=====================\n");
		printf("case1:rnd[%d] (%d,%d)\n", d, rnd[d].x, rnd[d].y);
		printf("direction (%d,%d)->(%d,%d)\n",
			   next_grid.gx, 
			   next_grid.gy, 
			   next_grid.gx + rnd[d].x,
			   next_grid.gy + rnd[d].y);
		printf("=====================\n");

		this->bus_change_direction(next_grid.gx + rnd[d].x , next_grid.gy + rnd[d].y);
		
		for (int i = 0 ; i < d ; i++){ // q_stateは、action が 0のものだから
		  q_state = q_state->next;
		}
		add_state_seq(q_state);  
		return 0;
	  }
	  else{  // ε:0.3以上
		STATE* s = q_state;
		int q_max = -1;
		int a;
		for (int i = 0; i < 5; i++){
		  if (q_max < s->q){
			q_max = s->q;
			a = s->action_type;
		  }
		  s = s->next;
		}
		

		printf("\n=====================\n");
		printf("case2:rnd[%d] (%d,%d)\n", a, rnd[a].x, rnd[a].y);
		printf("direction (%d,%d)->(%d,%d)\n",
			   next_grid.gx, 
			   next_grid.gy, 
			   next_grid.gx + rnd[a].x,
			   next_grid.gy + rnd[a].y);
		printf("=====================\n");

		this->bus_change_direction(next_grid.gx + rnd[a].x , next_grid.gy + rnd[a].y);

		for (int i = 0 ; i < a ; i++){ // q_stateは、action が 0のものだから
		  q_state = q_state->next;
		}
		add_state_seq(q_state); // q_stateは、action が 0のものだから

		return 0;
	  }
	}
  }

  return -1; // ここにきたらエラー
}

int BUS::driver()
{
  if (bus_status == 1){ // バスが動いている
	
	// まず、1ステップ分(3.6秒)を進める
	// const double diff_x = 0.00276; // X方向に250メートル
	// const double diff_y = 0.00225; // Y方向に250メートル
	// 1ステップに10m 進む → 時速10km だから、上の値を"25"で割れば良い
	// → (10.0/250.0)を乗算する
	
	double rad = atan2((next_bus_stop.py - present.py), 
					   (next_bus_stop.px - present.px));
	// presentを更新する
	present.py += diff_y * (10.0/250.0) * sin(rad);
	present.px += diff_x * (10.0/250.0) * cos(rad);
	
	double rad_0 = atan2((next_bus_stop.py - prev_bus_stop.py),
						 (next_bus_stop.px - prev_bus_stop.px));
	
	double rad_1 = atan2((next_bus_stop.py - present.py), 
						 (next_bus_stop.px - present.px));

	// 停留所を越えた // 完全直行の場合に判断ができないので、若干の誤差を入れる
	if (fabs(rad_0 -rad_1) >= pi * 0.5 - 0.0001){ 
	
	  bus_status = 0; // バスを停車させる
	  
	  // バス停に位置を固定する
	  present.py = next_bus_stop.py;
	  present.px = next_bus_stop.px;
	}
  }

  else if (bus_status == 0){ // バスが停車している
	stop_timer -= 1; // ステップ時間を1つ減算
	
	if ( stop_timer < 0){ // ステップ時間を使い果した

	  //printf("\n bus:bus stop\n");
	  
	  bus_status = 1; // バスを動かす
	  stop_timer = 5; // ステップ数を戻す

#if 1
	  ///// 2019/04/02 追記 ///////
	  // 待客ゼロ、乗客ゼロの場合は、リストを削除して、バスを強制停止させる
	  // 無駄に走っているのは勿体ないので
	  
	  // フラグ 
	  int d_flag1 = 0; // 待客ゼロ
	  int d_flag2 = 0; // 乗客ゼロ
	  
	  list<PERSON>::iterator pos;
	  for(pos = person_list.begin(); pos!=person_list.end(); ++pos){
		// バス乗車中(3)ではなく
		if (pos->person_status != 3){
		  // 到着(-1)もしていない
		  if (pos->person_status != -1){ 
			// 歩行中(0)でもない
			if (pos->person_status != 0){ 
			  d_flag1 = 1;  // バス待ちの乗客あり
			}
		  }
		}
	  }
	  
	  if (passenger.empty() == 0){// 乗客ゼロではない
		d_flag2 = 1; // 乗客あり
	  }
	  
	  if ((d_flag1 + d_flag2) == 0){ 
		p_bus_only_path.clear(); // ルートを削除する
		
		bus_status = 0; // バスを動かさない		
		
		// printf("name:%s, ***pass**\n",name);

		return 0;
	  }

	  
#endif //1

	  
	  // 反転制御開始
	  //if ((next_grid.gx == 8) && (next_grid.gy == 2)){
	  //direction = 2;
	  //printf("direction = 2\n");
	  //}
	  //else if ((next_grid.gx == 0) && (next_grid.gy == 2)){
	  //	direction = 1;		
	  //	printf("direction = 1\n");
	  //}
	  
	  // 周辺状況を把握を開始する
	  // フィールドに存在している、バスを待機している全ての乗客の情報(位置とステータス)
	  // を調べる
	  
	  // 自分の現在位置(座標)は、next_grid.gx, next_grid.gy
	  // 進行方向は、→は1 ←は2
	  
	  shortest = 999.9;
	  bus_flag = -1;
	  
	  _temp_enforced_pickup_person_number = -1; // (仮)乗客番号の退避
	  _temp_enforced_pickup_gx = -1; // (仮)強制乗車ポイント(gx)
	  _temp_enforced_pickup_gy = -1; // (仮)強制乗車ポイント(gx)
	  
	  _temp_enforced_dropoff_person_number = -1; // (仮)乗客番号の退避
	  _temp_enforced_dropoff_gx = -1; // (仮)強制乗車ポイント(gx)
	  _temp_enforced_dropoff_gy = -1; // (仮)強制乗車ポイント(gx)
	  
	  // if (direction == 1){ 	  // 進行方向は、→は1
	  if (direction == -1){ 	  // 進行方向は「無指向:-1」
		
		//絶対通過点(歩かせた乗客は必ず拾うは、次の検討にする)
		
		// 終端座標は(8,2)
		//shortest = d_bus_only[next_grid.gx * 10 + next_grid.gy][ 8 * 10 + 2]; 
		// dropoffを優先する為、最終座標計算は、(恣意的に)若干時間を長くする
		//shortest += 0.025 / 10.0;
		
		//n_gx = 8;
		//n_gy = 2;
		//bus_flag = 1;
		
		
		/////////////////////////
		// dropoffポイントを計算する
		// (dropoffはpickupより優先する(shortestを先行する))
		/////////////////////////
		
		list<PERSON>::iterator bp;
		for(bp = passenger.begin(); bp!=passenger.end(); ++bp){	// バスに乗っている乗客
		  if (bp->person_status == 3){ // 3:乗車中
			// _arr_xx は意味が変っているが、変更すると面倒になるのでこのまま
			
			for (int i = 0; i < 1; i++){
			  _arr_gx = bp->arr_grid.gx + rnd[i].x; 
			  _arr_gy = bp->arr_grid.gy + rnd[i].y;
			  
			  // 境界をチェックする ( (0 <= _arr_gx <= 8) and (0 <= _arr_gy <= 4))
			  //if ((0 <= _arr_gx) && (_arr_gx <= 8) && (0 <= _arr_gy) && (_arr_gy <= 4)){
			  if ((0 <= _arr_gx) && (_arr_gx <= 9) && (0 <= _arr_gy) && (_arr_gy <= 9)){
				//自分より前方しか見ない(というのは止める)
				//if (_arr_gx >=next_grid.gx){ 
				if (shortest > d_bus_only[next_grid.gx * 10 + next_grid.gy][_arr_gx * 10 + _arr_gy]){
				  shortest = d_bus_only[next_grid.gx * 10 + next_grid.gy][_arr_gx * 10 + _arr_gy];
				  n_gx = _arr_gx;
				  n_gy = _arr_gy;
				  bus_flag = 1;
				  
				  // 確定したらバスから、指定の乗客に、降車地点を教える
				  // 指定された乗客は、この地点を、dropoffとして自分で登録する
				  _temp_enforced_dropoff_person_number = bp->number; // (仮)乗客番号の退避
				  _temp_enforced_dropoff_gx = n_gx; // (仮)強制乗車ポイント(gx)
				  _temp_enforced_dropoff_gy = n_gy; // (仮)強制乗車ポイント(gx)
				}
				//}
			  }
			}
		  }
		}

		// 強制ドロップオフでひっかかっているはず
		if (_temp_enforced_dropoff_person_number != -1){ 
		  // passengerの方ではなくて、本家(グローバル)のイテレータを使う
		  
		  list<PERSON>::iterator pos;
		  for(pos = person_list.begin(); pos!=person_list.end(); ++pos){ 
			if (pos->number == _temp_enforced_dropoff_person_number){
			  // ドロップオフポイントを上書き
			  pos->dropoff_grid.gx = _temp_enforced_dropoff_gx; 
			  pos->dropoff_grid.gy = _temp_enforced_dropoff_gy;
			}
		  }
		}

		/////////////////////////
		// pickupポイントを計算する
		// pickupポイントは、dropoffより値が悪ければ、更新されない
		/////////////////////////
		
		list<PERSON>::iterator pos;
		// バスを待っている乗客
		for(pos = person_list.begin(); pos!=person_list.end(); ++pos){ 
		  //		  if (pos->person_status == 2){ // 2:バス待機中
		  if (pos->person_status == -2){ // -2:自宅
			// _dep_xx は意味が変っているが、変更すると面倒になるのでこのまま
			for (int i = 0; i < 1; i++){
			  _dep_gx = pos->dep_grid.gx + rnd[i].x; 
			  _dep_gy = pos->dep_grid.gy + rnd[i].y; 
			  
			  // 境界をチェックする ( (0 <= _dep_gx <= 8) and (0 <= _dep_gy <= 4))
			  //if ((0 <= _dep_gx) && (_dep_gx <= 8) && (0 <= _dep_gy) && (_dep_gy <= 4)){
			  if ((0 <= _dep_gx) && (_dep_gx <= 9) && (0 <= _dep_gy) && (_dep_gy <= 9)){
				
				// 自分より前方しか見ない に加えて、
				// 乗車前なので乗客の方向は気にする(を止める)

				//if ((_dep_gx >=next_grid.gx) && 
				//((direction == pos->direction) || (pos->direction == 3))) {
				  
				// バスの(next_grid.gx,next_grid.gy)→(_dep_gx, _dep_gy)の移動時間は
				// 以下の通り
				double _bus_drive_time = 
				  d_bus_only[next_grid.gx * 10 + next_grid.gy][_dep_gx * 10 + _dep_gy];
				
				// 乗客の(pos->dep_grid.gx, pos->dep_grid.gy)→(_dep_gx, _dep_gy)の移動時間は
				// 以下の通り
				double _person_walk_time = 
				  d_walk_only[pos->dep_grid.gx * 10 + pos->dep_grid.gy][_dep_gx * 10 + _dep_gy];
				
				// 当然のことながら、乗客の方が早く到着していなければ、バスに乗れない
				// これに該当しない座標は、検討対象から排除することになる
				if (_person_walk_time < _bus_drive_time){
				  
				  if (shortest > d_bus_only[next_grid.gx * 10 + next_grid.gy][_dep_gx * 10 + _dep_gy]){
					shortest = d_bus_only[next_grid.gx * 10 + next_grid.gy][_dep_gx * 10 + _dep_gy];
					n_gx = _dep_gx;
					n_gy = _dep_gy;
					bus_flag = 1;
					
					// 確定したらバスから、指定の乗客に、乗車地点を教える
					// 指定された乗客は、この地点を、pickupとして自分で登録する
					_temp_enforced_pickup_person_number = pos->number; // (仮)乗客番号の退避
					_temp_enforced_pickup_gx = n_gx; // (仮)強制乗車ポイント(gx)
					_temp_enforced_pickup_gy = n_gy; // (仮)強制乗車ポイント(gx)
					
				  }
				}
				// }			
			  }
			}
		  }
		}
		
		// 以下は発動されない可能性もある(shortestの成果が悪い場合)
		if (_temp_enforced_pickup_person_number != -1){
		  for(pos = person_list.begin(); pos!=person_list.end(); ++pos){ 
			if (pos->number == _temp_enforced_pickup_person_number){
			  // ドロップオフポイントを上書き
			  pos->pickup_grid.gx = _temp_enforced_pickup_gx; 
			  pos->pickup_grid.gy = _temp_enforced_pickup_gy;
			}
		  }
		}
	  }// 	  if (direction == -1){ 	  // 進行方向は、無指向
	  
	  /////////////////////////////////////////////////////////////////////////////////
	  

	  // 新ルートの生成
	  //// n_gx, n_gy にshortestが最小となるターゲットの座標は記録されているはず
	  // というのは訂正。
	  // 更新後にエントリーは消えるので、やっぱりダメ
	  // 再検索します。面倒だけど
	  
	  // まず最初に、dropffから調べる
	  // でもdropoffは、乗客の中から選ばないといけないので、ちょっと迂遠になる。
	  
	  shortest = 999.9; // リセットする
	  bus_flag = -1;
	  
	  //list<PERSON>::iterator pos;
	  for(pos = person_list.begin(); pos!=person_list.end(); ++pos){ // 乗客全員
		
		list<PERSON>::iterator bp;
		for(bp = passenger.begin(); bp!=passenger.end(); ++bp){	// バスに乗っている乗客
		  if (pos->number == bp->number){

			if (pos->dropoff_grid.gx != -1){
			
			  int _g1 = next_grid.gx * 10 + next_grid.gy;
			  int _g2 = pos->dropoff_grid.gx * 10 + pos->dropoff_grid.gy;
			  
			  if (shortest > d_bus_only[_g1][_g2]){
				shortest = d_bus_only[_g1][_g2];
				n_gx = pos->dropoff_grid.gx;
				n_gy = pos->dropoff_grid.gy;
				bus_flag = 1;
			  }
			}//if (pos->dropoff_grid.gx != -1){
		  }
		}

		//		if (pos->pickup_grid.gx != -1){
		if ((pos->pickup_grid.gx != -1) && (pos->on_the_bus == -1)) {
		  
		  int _g1 = next_grid.gx * 10 + next_grid.gy;
		  int _g2 = pos->pickup_grid.gx * 10 + pos->pickup_grid.gy;
		  
		  if (shortest > d_bus_only[_g1][_g2]){
			shortest = d_bus_only[_g1][_g2];
			n_gx = pos->pickup_grid.gx;
			n_gy = pos->pickup_grid.gy;
			bus_flag = 1;
		  }
		}// 		if (pos->pickup_grid.gx != -1){
	  } // 	  for(pos = person_list.begin(); pos!=person_list.end(); ++pos){ // 乗客全員

	  int g1 = next_grid.gx * 10 + next_grid.gy;
	  int g2 = n_gx * 10 + n_gy;

#if 0	// 目的地と現在地が一致してしまうこともある  
	  if (g1 == g2){
		printf("i = %d\n",global_clock);
		exit(1);
	  }
#endif
		
	  //  if (bus_flag == 1){
	  if ((bus_flag == 1) && (g1 != g2)){
		p_bus_only_path.clear(); // ルートを削除する

		//int g1 = next_grid.gx * 10 + next_grid.gy;
		//int g2 = n_gx * 10 + n_gy;

		printf("g1:%d, g2:%d\n", g1, g2);
		
		copy(path_bus_only[g1][g2].begin(), path_bus_only[g1][g2].end(), 
			 back_inserter(p_bus_only_path));
		
		p_bus_only_path.pop_front(); // 先頭の要素を削除	
		
#if 1
		printf("path2 ");
		list<int>::iterator pos;
		for(pos = p_bus_only_path.begin(); pos!=p_bus_only_path.end(); ++pos){
		  cout << *pos << " ";
		}
		printf("\n");
#endif
		
	  }

	  printf("name:%s: bus_flag=%d\n", name, bus_flag);

	  if (!p_bus_only_path.empty()){ // 空でなければ
		list<int>::iterator pos;
		pos = p_bus_only_path.begin();
		int new_x = (*pos) / 10;
		int new_y = (*pos) % 10;
		
		bus_change_direction(new_x, new_y);
		
#if 1
		//list<int>::iterator pos;
		printf("path3 ");
		for(pos = p_bus_only_path.begin(); pos!=p_bus_only_path.end(); ++pos){
		  cout << *pos << " ";
		}
		printf("\n");
#endif		
		p_bus_only_path.pop_front(); // (次の準備に為に)先頭の要素を削除	
		
	  }
	  else{ // 空であれば
		// printf("Bus Path is exhausted\n");
		// exit(1); // 強制終了してしまいましょう。
	  }
	}
  } // else if (bus_status == 0){

  return 0;
}

void BUS::add_state_seq(STATE *state)
{
  printf("start add_state_seq()\n");

  STATE_SEQ *new_p_state_seq = (STATE_SEQ *)malloc(sizeof(STATE_SEQ));
  if(new_p_state_seq == NULL) {
	printf("メモリが確保できません\n");
	exit(EXIT_FAILURE);
  }
  memset(new_p_state_seq, 0, sizeof(STATE_SEQ)); // 
  memcpy(new_p_state_seq, &present_state_seq, sizeof(STATE_SEQ)); // 現在の「状態」を入力
 
  // 適当なstateを作って貼りつけておく
  //new_p_state_seq->state = make_state();
  new_p_state_seq->state = state;

  // 暫定的なテスト(ここまで)
  

  STATE_SEQ *p_state_seq = p_last_state_seq->prev_seq;

  p_state_seq->next_seq = new_p_state_seq;
  new_p_state_seq->prev_seq = p_state_seq;

  p_last_state_seq->prev_seq = new_p_state_seq;
  new_p_state_seq->next_seq = p_last_state_seq;

  printf("end add_state_seq()\n");

  return;
}


void BUS::del_state_seq()
{
  STATE_SEQ* p_state_seq = p_first_state_seq->next_seq;  
  while (p_state_seq != p_last_state_seq){  
	STATE_SEQ* p_seq = p_state_seq->next_seq; // ポインタを回避
	free(p_state_seq);
	p_state_seq = p_seq;
  }
  // ポインタのリセット
  p_first_state_seq->prev_seq = NULL;
  p_last_state_seq->next_seq = NULL;
  p_first_state_seq->next_seq = p_last_state_seq;
  p_last_state_seq->prev_seq = p_first_state_seq;
}


void BUS::q_reward(int final_reward)
{
  printf("passed q_reward(%d)\n", final_reward);
  
  p_last_state_seq->state->q = final_reward; // ゴールに金額をぶらさげておく

  int number = 0;
  int f_r = final_reward;
  
  STATE_SEQ* p_state_seq = p_last_state_seq->prev_seq;  
  while (p_state_seq != p_first_state_seq){  
	/// 処理内容(開始)
	/// 逆方向に報酬を与えていく

#if 1	
	p_state_seq->state->q += 
	  0.1 * (0.9 * p_state_seq->next_seq->state->q - p_state_seq->state->q);



	//p_state_seq->state->q += 
	//  0.9 * p_state_seq->state->q + 0.1 * p_state_seq->next_seq->state->q ;


#else
	p_state_seq->state->q += 10; // 無条件に10点を与える、としたら？

	if (f_r > 0) {
	  p_state_seq->state->q  += f_r;
	  f_r -= 1;
	}
#endif

	printf("%d ", number);


	printf("===========\n");
	printf("number:%d\n",p_state_seq->state->number);
	printf("action_type:%d\n",p_state_seq->state->action_type);
	printf("q:%d\n",p_state_seq->state->q);
	printf("===========\n");

	number += 1;
	p_state_seq = p_state_seq->prev_seq;
  }

  printf("total state number = %d\n", number);
}
 
  
void BUS::init_state_seq_list()
{
  STATE_SEQ *p_top_state_seq = (STATE_SEQ *)malloc(sizeof(STATE_SEQ));
  if(p_top_state_seq == NULL) {
	printf("メモリが確保できません\n");
	exit(EXIT_FAILURE);
  }
  memset(p_top_state_seq, 0, sizeof(STATE_SEQ)); // ゼロクリア

  // 暫定的なテスト(ここから)

  // 適当なstateを作って貼りつけておく
  p_top_state_seq->state = make_state(0,0,0);

  // 暫定的なテスト(ここまで)

  STATE_SEQ *p_tail_state_seq = (STATE_SEQ *)malloc(sizeof(STATE_SEQ));
  if(p_tail_state_seq == NULL) {
	printf("メモリが確保できません\n");
	exit(EXIT_FAILURE);
  }
  memset(p_tail_state_seq, 0, sizeof(STATE_SEQ)); // ゼロクリア


  // 暫定的なテスト(ここから)

  // 適当なstateを作って貼りつけておく
  p_tail_state_seq->state = make_state(0,0,0);

  // 暫定的なテスト(ここまで)

  p_first_state_seq = p_top_state_seq;
  p_last_state_seq = p_tail_state_seq;

  p_first_state_seq->prev_seq = NULL;
  p_last_state_seq->next_seq = NULL;
  p_first_state_seq->next_seq = p_last_state_seq;
  p_last_state_seq->prev_seq = p_first_state_seq;

  return;
}
