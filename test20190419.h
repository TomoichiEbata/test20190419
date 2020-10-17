/*
  test20190419.h
*/

#ifndef _CA_SMALL_H_
#define _CA_SMALL_H_

// 実は江端の家の座標
const double center_x = 139.4755406;
const double center_y =  35.598697;

const double diff_x = 0.00276; // X方向に250メートル
const double diff_y = 0.00225; // Y方向に250メートル

const double pi = 3.141592654;

struct RND // roundの意味
{
  int x;
  int y;
};

struct STATE
{
  int number;
  int action_type;
  int q;

  STATE *prev;
  STATE *next;
};

struct STATE_SEQ
{
  STATE *state;  // state_listのどれか一つを参照する

  STATE_SEQ *prev_seq;
  STATE_SEQ *next_seq;

};


class LOCATION // 実座標
{
public:
  double px;
  double py;
};

class GRID // 方眼座標
{
public:
  int gx;
  int gy;
};


class BUS;

class PERSON
{
public:
  STATE_SEQ *p_first_state_seq, *p_last_state_seq;  
  STATE_SEQ *present_state_seq; 
    
  int direction; // 1:→ 2:← 3:なし

  int pickup_flag; // デフォルトは0   
  int person_flag; 

  int stop_timer;
  int person_status; // 0:出発前 1:歩行中 2:バス待機中 3:乗車中 -1:到着完了(停止中)
  
  int number;
  char name[10]; // 名称 例 "person5" "person15"
  
  GRID dep_grid; // 出発地(方眼座標)
  GRID arr_grid; // 到着地(方眼座標) 

  GRID pickup_grid; // バスのピックアップ地(方眼座標)
  GRID dropoff_grid; // バスのピックアップ地(方眼座標)
  
  LOCATION departure; // 出発地
  LOCATION arrival; // 到着地
  
  GRID prev_grid; // 前の交差点(方眼座標)
  GRID next_grid; // 次の交差点(方眼座標) 
  
  LOCATION prev_person_stop; // 前の交差点(緯度、軽度)
  LOCATION present;  // 現在位置(緯度、軽度)
  LOCATION next_person_stop; //次の交差点(緯度、軽度)
  
  BUS *bus; // 現在乗車しているバス

  int on_the_bus; // 現在乗車しているバス(の番号)
  
  list<int> p_walk_and_bus_path; // 歩行とバスを使った場合の経路
  list<int> p_walk_only_path; // 歩いた場合の経路

  int gen_time; // 発生時刻
  int dep_time; // 出発時刻
  int home_time;  // 自宅待機時間
  int arr_time; // 到着時刻
  int travel_time; // 移動時間

  int life_time; // 発生から消滅までの時間

  PERSON(int num,
		 const char* n, 
		 int dep_grid_gx, int dep_grid_gy, int arr_grid_gx, int arr_grid_gy);
  
  int send_udp();
  int person_change_direction(int next_grid_gx, int next_grid_gy);

  int walk_only_check();
  int walk_and_bus_check(list<BUS>::iterator bus);

  void init_state_seq_list();
  void add_state_seq();
  void q_reward(int final_reward);
  

};


class BUS
{
 public:
  STATE_SEQ *p_first_state_seq, *p_last_state_seq;  
  STATE_SEQ *present_state_seq; 
  
  
  char name[10] = {0};

  int number; // バスのユニークな番号

  int direction; // 1:→ 2:←
  
  GRID prev_grid; // 前のバス停留所(方眼座標)
  GRID next_grid; // 次のバス停留所(方眼座標) 

  LOCATION prev_bus_stop; // 前のバス停留所(緯度、軽度)
  LOCATION present;  // 現在位置(緯度、軽度)
  LOCATION next_bus_stop; //次のバス停留所(緯度、軽度)
  
  int stop_timer;
  int bus_status;

  list<int> p_bus_only_path; // バスを使った場合の経路

  list<PERSON> passenger;  // そのバスに乗っている乗客のリスト

  double shortest;
  int bus_flag; 
  int n_gx, n_gy, _arr_gx, _arr_gy, _dep_gx, _dep_gy;

  int _temp_enforced_pickup_person_number;
  int _temp_enforced_pickup_gx;
  int _temp_enforced_pickup_gy;

  int _temp_enforced_dropoff_person_number;
  int _temp_enforced_dropoff_gx;
  int _temp_enforced_dropoff_gy;

  //// 12状態にまで圧縮 (近くにいる状態が一番重いと判断した)
  // "3"の意味→ Waiting Person | Riden Person | Bus. 
  // "4"の意味 -> 45～135度 | 135～225度 | 225 ～ 315度 | 315 ～ 45度
  // 値の意味3:360メートル以内、2: 720メートル以内、1:1080メール以内 0: いない
  int field_check[3][4] = {0} ; 
  
  BUS(const char *n, int prev_grid_gx, int prev_grid_gy, int route_case);
  int send_udp();
  int bus_change_direction(int next_grid_gx, int next_grid_gy);
  int driver();
  int driver_q();
  int check_environment();

  void init_state_seq_list();
  void add_state_seq(STATE *s);
  void q_reward(int final_reward);
  void del_state_seq();

};

//// d[100][100]の退避変数



#endif // _CA_SMALL_H_
