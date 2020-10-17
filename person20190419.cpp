/*

  g++ -c person20190419.cpp

  person_statusの説明

  // person_status = -2; // 自宅
  // person_status = -1; // 永久停止 
  // person_status = 0; // 停止:0
  // person_status = 1; // 0歩行:1 
  // person_status = 2; // バス待機中
  ((person_status == 3) && (bus->number == on_the_bus)){ // バス乗車中

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


list<PERSON> person_list; // シミュレータ上に存在する全ての人間
extern LOCATION bus_stop[10][10];
extern int global_clock; // main の中のint iと同じ値にする

extern int sd;
extern struct sockaddr_in addr;
extern WSADATA wsaData;


PERSON::PERSON(int num,
			   const char* n, 
			   int dep_grid_gx, int dep_grid_gy, int arr_grid_gx, int arr_grid_gy) 
{
  gen_time = global_clock; // main の int iの値を入れる

  // person_status = 0; // 歩行:1 停止:0
  // person_status = 2; // 最初からバスを待つ

#if 0 // Q学習の時は、ここを"0"にする
  person_status = -2; // 自宅(-2)を新設
#else 
  person_status = 2; // (いきなり)バス待機中
#endif

  dep_time = -1;
  home_time = -1;
  arr_time = -1; 
  travel_time = -1;
  life_time = -1;

#if 0
  if ( arr_grid_gx > dep_grid_gx) 
	direction = 1; // →方向
  else if ( arr_grid_gx < dep_grid_gx)
	direction = 2; // ←方向
  else 
	direction = 3; // ↑↓方向
#endif //0 
  
  direction = -1; // 無指向

  on_the_bus = -1; // 最初はバスとのリンクはない
  
  pickup_flag = 0; // デフォルトは0   
  person_flag = 0; // フラグは0で上げておく
  
  number = num; // 乗客番号
  
  //bus = b;  // ポインタでリンク
  
  strcpy(name, n); //名前のコピー
  
  stop_timer = 2; // 交差点のステップ数
  
  // 先ず出発地と到着地のグリッドと座標を決定
  dep_grid.gx = dep_grid_gx;
  dep_grid.gy = dep_grid_gy;
  arr_grid.gx = arr_grid_gx;
  arr_grid.gy = arr_grid_gy;
  
  departure.px = bus_stop[dep_grid_gx][dep_grid_gy].px;
  departure.py = bus_stop[dep_grid_gx][dep_grid_gy].py;
  arrival.px = bus_stop[arr_grid_gx][arr_grid_gy].px;
  arrival.py = bus_stop[arr_grid_gx][arr_grid_gy].py;
  
  // 交差点の初期値を固定
  prev_grid.gx = next_grid.gx = dep_grid_gx;
  prev_grid.gy = next_grid.gy = dep_grid_gy;
  
  present.px = prev_person_stop.px = next_person_stop.px 
	= bus_stop[dep_grid_gx][dep_grid_gy].px;
  present.py = prev_person_stop.py = next_person_stop.py 
	= bus_stop[dep_grid_gx][dep_grid_gy].py;
 
  // ピックアップポイントとドロップオフポイントのデフォルト設定(-1,-1)

#if 0 // Q学習の時は、ここを"0"にする
  pickup_grid.gx = -1;
  pickup_grid.gy = -1;
  dropoff_grid.gx = -1;
  dropoff_grid.gy = -1;
#else  
  pickup_grid.gx = dep_grid_gx;
  pickup_grid.gy = dep_grid_gy;
  dropoff_grid.gx = arr_grid_gx;
  dropoff_grid.gy = arr_grid_gy;
#endif // 0

}


int PERSON::send_udp()
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


int PERSON::person_change_direction(int next_grid_gx, int next_grid_gy)
{ // 次の経路を示す
  prev_grid.gx = next_grid.gx;
  prev_grid.gy = next_grid.gy;
  
  next_grid.gx = next_grid_gx;
  next_grid.gy = next_grid_gy;
	
  prev_person_stop.px = next_person_stop.px;
  prev_person_stop.py = next_person_stop.py;	
  next_person_stop.px = bus_stop[next_grid.gx][next_grid.gy].px;
  next_person_stop.py = bus_stop[next_grid.gx][next_grid.gy].py;

  return 0;
}

// status 1, -1, 0,-2 の4状態を扱う
int PERSON::walk_only_check()
{
  //bus = b;  // ポインタでリンク	
  /////////////////////////////////////////////////////////////////////////////  
  if (person_status == 1){ // 1:歩行中
	/////////////////////////////////////////////////////////////////////////////
	
	// まず、1ステップ分(3.6秒)を進める
	// const double diff_x = 0.00276; // X方向に250メートル
	// const double diff_y = 0.00225; // Y方向に250メートル
	// 1ステップに3.6m 進む → 時速3.6km だから、
	// → (3.6 / 250.0)を乗算する
	
	double rad = atan2((next_person_stop.py - present.py), 
					   (next_person_stop.px - present.px));
	// presentを更新する
	present.py += diff_y * (3.6 / 250.0) * sin(rad);
	present.px += diff_x * (3.6 / 250.0) * cos(rad);
	
	double rad_0 = atan2((next_person_stop.py - prev_person_stop.py),
						 (next_person_stop.px - prev_person_stop.px));
	
	double rad_1 = atan2((next_person_stop.py - present.py), 
						 (next_person_stop.px - present.px));
	
	
	// ■ステータスチェンジの切っ掛け
	if (fabs(rad_0 -rad_1) >= pi * 0.5 - 0.0001){ // 交差点を越えた
	  
	  // 交差点に位置を固定する
	  present.py = next_person_stop.py;
	  present.px = next_person_stop.px;
	  
	  // 一時停止(0)にチェンジ
	  person_status = 0;
	  printf("\nnum=%d:person_status = 1->0\n", number);
	} // f (fabs(rad_0 -rad_1) >= pi * 0.5 - 0.0001){ // 交差点を越えた
  } // if (person_status == 1){ // 1:歩行中

  /////////////////////////////////////////////////////////////////////////////  
  else if (person_status == -1){ // 永久停止 
  /////////////////////////////////////////////////////////////////////////////
	if (arr_time == -1){ // 到着時刻を記録
	  arr_time = global_clock;
	  travel_time = arr_time - dep_time;
	}

	person_status = -1; // 永久停止からは抜けられない
	// リストから外す // サブルーチンの外で削除する

	//printf("\n -1 -> -1\n"); 
  } // status == -1 

  /////////////////////////////////////////////////////////////////////////////  
  else if (person_status == -2){ // 自宅
  /////////////////////////////////////////////////////////////////////////////
	if ((pickup_grid.gx != -1) && (dropoff_grid.gx == -1)) {
	  
	  if (((next_grid.gx == dep_grid.gx) && (next_grid.gy == dep_grid.gy)) ||
		  ((next_grid.gx == pickup_grid.gx) && (next_grid.gy == pickup_grid.gy))){
		
		if ((pickup_grid.gx == dep_grid.gx) && (pickup_grid.gy == dep_grid.gy)){
		  person_status = 2;
		  printf("\nnum=%d:person_status = -2->2\n", number);
		}
		else{
		  person_change_direction(pickup_grid.gx, pickup_grid.gy);
		  person_status = 1;
		  printf("\nnum=%d:person_status = -2->1\n", number);
		}
	  }
	}
  }		

  /////////////////////////////////////////////////////////////////////////////  
  else if (person_status == 0){ // 一時停止
  /////////////////////////////////////////////////////////////////////////////
	stop_timer -= 1; // ステップ時間を1つ減算
	
	if ( stop_timer < 0){ // ステップ時間を使い果したらステータスが動き出す
	  
	  stop_timer = 2; // ステップ数を戻す
	  // 現在位置をnext_grid.gx,next_grid.gy
	  
	  // ピックアップが登録されているがドロップオフは登録されていない
	  if ((pickup_grid.gx != -1) && (dropoff_grid.gx == -1)){
		if ((next_grid.gx == pickup_grid.gx) && (next_grid.gy == pickup_grid.gy)){
		  person_status = 2;
		  printf("\nnum=%d:person_status = 0->2\n", number);
		}
	  }

	  // ピックアップ、ドロップオフが登録されていて最終到着値なら永久停止
	  if ((pickup_grid.gx != -1) && (dropoff_grid.gx != -1)) {
		if ((next_grid.gx == arr_grid.gx) && (next_grid.gy == arr_grid.gy)){// 到着
		  person_status = -1;
		  printf("\nnum=%d:person_status = 0->-1\n", number);
		  printf("next_grid.gx=%d, arr_grid.gx=%d next_grid.gy=%d,arr_grid.gy=%d\n",
				 next_grid.gx, arr_grid.gx,next_grid.gy,arr_grid.gy);
		}
	  }

	  // ピックアップ、ドロップオフが登録されてが最終到着でないなら歩く
	  if ((pickup_grid.gx != -1) && (dropoff_grid.gx != -1)) {
		if ((next_grid.gx != arr_grid.gx) || (next_grid.gy != arr_grid.gy)){//未到着
		  person_status = 1;
		  person_change_direction(arr_grid.gx,arr_grid.gy);
		  printf("\nnum=%d:person_status = 0->1\n", number);
		  printf("next_grid.gx=%d, arr_grid.gx=%d next_grid.gy=%d,arr_grid.gy=%d\n",
				 next_grid.gx, arr_grid.gx,next_grid.gy,arr_grid.gy);
		  
		}
	  }
	}
  } //   else if (person_status == 0){ // 一時停止

  return 0;
}

// status 2, 3 の2状態を扱う
int PERSON::walk_and_bus_check(list<BUS>::iterator bus)
{
  /////////////////////////////////////////////////////////////////////////////
  if (person_status == 2){ // バス待機中
  /////////////////////////////////////////////////////////////////////////////
	// このケースでは、乗客に自律歩行の意思はないとする
	// バスの方向性と、乗客の方向性が一致していれば、乗ればよい
	// 乗客に自由意思はない
	
	// バスの方向と移動方向が一致していて、
	if((bus->bus_status == 0) && (bus->direction ==  direction)){
	  // バスの位置と待機中の位置が一緒であれば
	  // バスの現在位置(座標)は、bus->next_grid.gx, bus->next_grid.gy
	  // 乗客の現在位置(座標)は、next_grid.gx, next_grid.gy
	  
	  printf("bus status = 2 Bus:%d,%d, Person:%d,%d\n",
			 bus->next_grid.gx,
			 bus->next_grid.gy,
			 next_grid.gx,
			 next_grid.gy);
	  
	  if ((bus->next_grid.gx == next_grid.gx) && (bus->next_grid.gy == next_grid.gy)){
		on_the_bus = bus->number; // バスとのリンクを張る


		//この段階ではdropoff_gridが未確定の可能性が高い(というか確実にそうだろう)
		// なので change_direction(dropoff_grid.gx, dropoff_grid.gy) は別のところでやる必要がある
		//change_direction(dropoff_grid.gx, dropoff_grid.gy);		

		person_status = 3;
		printf("\nnum=%d:person_status = 2->3\n", number);
		printf("bus_num=%d\n", bus->number);		

		// 最後に入れる
		bus->passenger.push_back(*this); // バスの乗客

	  }
	}
  } // status == 2
  	//else if (status == 3){ // バス乗車中
  /////////////////////////////////////////////////////////////////////////////
  else if ((person_status == 3) && (bus->number == on_the_bus)){ // バス乗車中
  /////////////////////////////////////////////////////////////////////////////

	if (dep_time == -1){
	  dep_time = global_clock;
	  home_time = dep_time - gen_time;
	}

	//static int flag = 0;  //初期フラグは倒しておく
	
	if (bus->bus_status == 1){ // バス移動中
	  // presentをバスの移動に連動させる
	  present.py = bus->present.py;
	  present.px = bus->present.px;
	  
	  // 移動中に、dropoff_gridが確定するはずなので、そのタイミングで(1回だけ)登録する
	  if ((dropoff_grid.gx != -1) && (pickup_flag == 0)){
		pickup_flag = 1;
		person_change_direction(dropoff_grid.gx, dropoff_grid.gy);		
		printf("dropoff_grid is entried %d:%d\n",dropoff_grid.gx, dropoff_grid.gy);
	  }

	  person_flag = 1; // フラグを立て続ける
	}// 	if (bus->bus_status == 1){ // バス移動中

	else if (bus->bus_status == 0){ // バス停車中(下車するバス停か否か)
	  // 1回だけ通過させるためにフラグを使う
	  if (person_flag == 1){
		//printf("bus and person stop \n");
		person_flag = 0; // フラグを倒す

		// バスの位置と乗客の降車位置が一致していれば、下車する
		// バスの現在位置(座標)は、bus->next_grid.gx, bus->next_grid.gy
		// 乗客の現在位置はarr_grid.gx, arr_grid.gy

#if 0
		// もし、この段階でも、dropoff_gridが決定していなかったら、バグ
		if (dropoff_grid.gx == -1){
		  printf("Bug; dropoff_grid is not fixed!");
		  exit(-1);
		}
#endif
		if (dropoff_grid.gx != -1){ // この段階で、dropoff_gridが決定していないこともある	
		  if ((bus->next_grid.gx == dropoff_grid.gx) && (bus->next_grid.gy == dropoff_grid.gy)){
			person_status = 0;		  
			
			//dropoff_flag = 1; // dropoff_flagを上げる
			
			printf("\nnum=%d:person_status = 3->0\n", number);
			printf("bus->next_grid.gx=%d, dropoff_grid.gx=%d bus->next_grid.gy=%d, dropoff_grid.gy=%d\n",bus->next_grid.gx,dropoff_grid.gx,bus->next_grid.gy, dropoff_grid.gy);
			printf("arr_grid.gx=%d arr_grid.gy=%d\n", arr_grid.gx, arr_grid.gy);
			printf("bus->number=%d\n",bus->number);
			
			on_the_bus = -1; // バスとのリンクを切る
			
			// バス乗客リストからの削除
			list<PERSON>::iterator bp = bus->passenger.begin();
			while( bp != bus->passenger.end()) {
			  if (bp->number == number){
				bp = bus->passenger.erase(bp);
				cout << "erased:" << number <<"\n";
			  }
			  else 
				bp++;
			}
		  }
		}

	  }// 	  if (person_flag == 1){
	}//	else if (bus->bus_status == 0){ // バス停車中(下車するバス停か否か)

	else {
	  printf("exit in 519\n");
	  exit(-1);
	}
  } //   else if ((person_status == 3) && (bus->number == on_the_bus)){ // バス乗車中
  return 0;
}
