/*

  wf.cppをベースに作ったソースコード → [9x9]に拡張

  g++ -c route99.cpp 
  でコンパイルチェックして、メインルーチンにリンクする(dllは面倒なのでやらない)

  しばらくはテストの為に、int main() をつけて
  g++ -g route99.cpp -o route99
  とする

  ■無理して汎用化を目指さない(どうせ忘れてしまう)
  ■パラメータは、直接数値を入れるようにして、なんでもクラスや構造体渡しにしない
  (デバッグで死ぬから)

  ■ノード番号は"0-99"までしか使えないでいい(どうせ実験なんだから)
  ■座標の番号は、 (x,y)=(8,7) なら、"87"番とする、という安易な決め方にする
  (従って、x,yともに、座標は、0,1,2,3,4,5,6,7,8,9 しか使えない)

  ■ノード番号がスパウス(スカスカ)になっても、気にしない
 
*/
 
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <list>   // list 利用のため

using namespace std;

//int d[100][100];  // d[i][k]：ノードiからノードkへの移動時間 
//int via[100][100];  // d[i][k]の間にある(少くとも1つの)中継ノード

double d[100][100];  // d[i][k]：ノードiからノードkへの移動時間 
int via[100][100];  //  d[i][k]の間にある(少くとも1つの)中継ノード

//list<int> path[100][100];   // int 型の list を宣言  
 

#if 1
// 中継パスの表示ルーチン(再帰呼出し用)
void printPath1_aux(int begin, int end) {
  if (via[begin][end] == begin) {
	if (begin != end)
	  printf("%02d -> ", begin);
	return;
  }
  
  printPath1_aux(begin, via[begin][end]);
  printPath1_aux(via[begin][end], end);
}
#endif

// 中継パスの表示ルーチン(再帰呼出し用)
void printPath1_aux(int begin, int end, list<int>* p) {
  if (via[begin][end] == begin) {
	if (begin != end){
	  // printf("%02d -> ", begin);
	  p->push_back(begin);
	}
	return;
  }
  
  printPath1_aux(begin, via[begin][end], p);
  printPath1_aux(via[begin][end], end, p);
}


 
// 中継パスの表示ルーチン
#if 1
void printPath1(int start, int goal) {
  printPath1_aux(start, via[start][goal]);
  printPath1_aux(via[start][goal], goal);
  printf("%02d\n", goal);
}
#endif 

void printPath1(int start, int goal, list<int> *p ) {
  printPath1_aux(start, via[start][goal], p);
  printPath1_aux(via[start][goal], goal, p);
  // printf("%02d\n", goal);
  p->push_back(goal);

}

int make_route_init(void)
{
  // 変数の初期化 (全部の要素を埋めているので、ゼロリセットする必要はない)
  for(int i = 0; i < 100; i++){
	for(int j = 0; j < 100; j++){
	  d[i][j] = 999.9; // 移動時間の初期化(非常識なくらいでっかい値を入力しておく(INT_MAXは足し算の時に桁上がりが起こるので使わない)
	  via[i][j] = i; // ノードiからノードkへの経由値の初期化 
	}
  }
  
  for(int i = 0; i < 100; i++){
	d[i][i] = 0; //// 同じノードへの移動時間は0になるので、上書き
  }
  
  //ノード番号の通番を以下のようにする
  // [0][2] → "02", [4][9] → "49", [9][[9] → "99"
  // 座標は1ケタ内に留める
  
  // 以下の0.069とは、250メートルを時速3.6kmで歩いた時の、時間(0.069時間,4.2分、250秒)のことである。
  
  //for (int y = 0; y < 5; y++){
  for (int y = 0; y < 10; y++){
	//for (int x = 0; x < 9; x++){
	for (int x = 0; x < 10; x++){
	  
	  int n_num = x * 10 + y;

	  // + ( 1, 0)
	  int x_new = x + 1;
	  int y_new = y;

	  if (x_new < 10){
		int n_num_next = x_new * 10 + y_new;
		d[n_num][n_num_next] = 0.069;
		
		//printf("1:d[%02d][%02d]=%f\n",n_num, n_num_next, d[n_num][n_num_next]);

	  }
	  // + (-1, 0)
	  x_new = x - 1;
	  y_new = y;

	  if (x_new > -1 ){
		int n_num_next = x_new * 10 + y_new;
		d[n_num][n_num_next] = 0.069;
		//printf("2:d[%02d][%02d]=%f\n",n_num, n_num_next, d[n_num][n_num_next]);
	  }

	  // + ( 0, 1)
	  x_new = x;
	  y_new = y + 1;
	  
	  if (y_new < 10 ){
		int n_num_next = x_new * 10 + y_new;
		d[n_num][n_num_next] = 0.069;
		//printf("3:d[%02d][%02d]=%f\n",n_num, n_num_next, d[n_num][n_num_next]);
	  }
	  
	  // + ( 0,-1)
	  x_new = x;
	  y_new = y - 1;
	  
	  if (y_new > -1 ){
		int n_num_next = x_new * 10 + y_new;
		d[n_num][n_num_next] = 0.069;
		//printf("4:d[%02d][%02d]=%f\n",n_num, n_num_next, d[n_num][n_num_next]);
	  }
	}
  }
  return 0;
}


int make_route_change_walk_only(void)
{
  make_route_init();
  return 0;
}

int make_route_change_walk_and_bus(void)
{
  // バスの移動時間 
  // 以下の0.025とは、250メートルを時速10kmで走行した時の、時間(0.025時間、1.5分、90秒)のことである。
  // 実験用上書き(ここがキモです)
  
  d[02][12] = 0.025;  
  d[12][22] = 0.025;  
  d[22][32] = 0.025;  
  d[32][42] = 0.025;  
  d[42][52] = 0.025;  
  d[52][62] = 0.025;  
  d[62][72] = 0.025;  
  d[72][82] = 0.025;  
  
  d[12][02] = 0.025;  
  d[22][12] = 0.025;  
  d[32][22] = 0.025;  
  d[42][32] = 0.025;  
  d[52][42] = 0.025;  
  d[62][52] = 0.025;  
  d[72][62] = 0.025;  
  d[82][72] = 0.025;  
  
  return 0;
  
}

int make_route_change_bus_only(void)
{
  // 変数の初期化 (全部の要素を埋めているので、ゼロリセットする必要はない)
  for(int i = 0; i < 100; i++){
	for(int j = 0; j < 100; j++){
	  d[i][j] = 999.9; // 移動時間の初期化(非常識なくらいでっかい値を入力しておく(INT_MAXは足し算の時に桁上がりが起こるので使わない)
	  via[i][j] = i; // ノードiからノードkへの経由値の初期化 
	}
  }
  
  for(int i = 0; i < 100; i++){
	d[i][i] = 0; //// 同じノードへの移動時間は0になるので、上書き
  }

  //ノード番号の通番を以下のようにする
  // [0][2] → "02", [4][9] → "49", [9][[9] → "99"
  // 座標は1ケタ内に留める

  //  for (int y = 0; y < 5; y++){
  for (int y = 0; y < 10; y++){
	//for (int x = 0; x < 9; x++){
	for (int x = 0; x < 10; x++){

	  int n_num = x * 10 + y;

	  // + ( 1, 0)
	  int x_new = x + 1;
	  int y_new = y;

	  // if (x_new < 9){
	  if (x_new < 10){
		int n_num_next = x_new * 10 + y_new;
		d[n_num][n_num_next] = 0.025;
		
		//printf("1:d[%02d][%02d]=%f\n",n_num, n_num_next, d[n_num][n_num_next]);

	  }
	  // + (-1, 0)
	  x_new = x - 1;
	  y_new = y;

	  if (x_new > -1 ){
		int n_num_next = x_new * 10 + y_new;
		d[n_num][n_num_next] = 0.025;
		//printf("2:d[%02d][%02d]=%f\n",n_num, n_num_next, d[n_num][n_num_next]);
	  }

	  // + ( 0, 1)
	  x_new = x;
	  y_new = y + 1;

	  // if (y_new < 5 ){
	  if (y_new < 10 ){
		int n_num_next = x_new * 10 + y_new;
		d[n_num][n_num_next] = 0.025;
		//printf("3:d[%02d][%02d]=%f\n",n_num, n_num_next, d[n_num][n_num_next]);
	  }

	  // + ( 0,-1)
	  x_new = x;
	  y_new = y - 1;

	  if (y_new > -1 ){
		int n_num_next = x_new * 10 + y_new;
		d[n_num][n_num_next] = 0.025;
		//printf("4:d[%02d][%02d]=%f\n",n_num, n_num_next, d[n_num][n_num_next]);
	  }
	}
  }

  return 0;

}


  
int make_route_calc(void)
{
#if 0  
  // 経路長計算
  for (int k =0; k < 99; k++){  
	for (int i =0; i < 99; i++){
	  for(int j = 0; j < 99; j++){
		if(d[i][j] > d[i][k] + d[k][j]){
		  d[i][j] = d[i][k] + d[k][j];
		  via[i][j] = k; //更新処理
		}
	  }
	}
  }
  return 0;
#endif 

  // 経路長計算
  for (int k =0; k < 100; k++){  
	for (int i =0; i < 100; i++){
	  for(int j = 0; j < 100; j++){
		if(d[i][j] > d[i][k] + d[k][j]){
		  d[i][j] = d[i][k] + d[k][j];
		  via[i][j] = k; //更新処理
		}
	  }
	}
  }
  return 0;


}

#if 0

int main()
{
  make_route_init();
  make_route_calc();

  // path_walkの2次元配列をそのままサブルーチンに持っていくのは難しいので、
  // メインで回すことにする(少々カッコ悪いが)

  list<int> path_walk[100][100];   // int 型の list を宣言  

  printf("\n[TRY1]\n");
  for(int i = 0; i < 100; i++){
	for(int k = 0; k < 100; k++){
	  if ((d[i][k] < 999.0) && (d[i][k] > 0.000001)) {
		printf("d[%02d][%02d]:%f ",i,k,d[i][k]);
		printPath1(i, k);
		printPath1(i, k, &(path_walk[i][k]));
	  }
	}
  }


  make_route_init();
  make_route_change(); // 路線内容の強制上書き
  make_route_calc();

  list<int> path_bus[100][100];   // int 型の list を宣言  

  printf("\n[TRY2]\n");
  for(int i = 0; i < 100; i++){
	for(int k = 0; k < 100; k++){
	  if ((d[i][k] < 999.0) && (d[i][k] > 0.000001)) {
		printf("d[%02d][%02d]:%f ",i,k,d[i][k]);
		printPath1(i, k);
		printPath1(i, k, &(path_bus[i][k]));
	  }
	}
  }

  // path_walk , path_busはマスタとして使って、実際のルートを弄るときは、
  // 以下の用にコピーして使うこと

  list<int> p_bus = path_bus[83][04];
  list<int> p_walk = path_walk[83][04];

  list<int> p_bus_1(p_bus.size());
  copy(p_bus.begin(), p_bus.end(), p_bus_1.begin());

  list<int> p_walk_1(p_walk.size());
  copy(p_walk.begin(), p_walk.end(), p_walk_1.begin());

  // イテレータ (反復子) の定義
  list<int>::iterator pos;
  for(pos = p_bus_1.begin(); pos!=p_bus_1.end(); ++pos){
      cout << *pos << "\n";
  }

  for(pos = p_walk_1.begin(); pos!=p_walk_1.end(); ++pos){
      cout << *pos << "\n";
  }


#if 0
  // https://cpprefjp.github.io/reference/algorithm/copy.html
  // back_inserter を使って l2 へ設定。
  // back_inserter は要素をコピーするときに l2.push_back() するイテレータを作る関数。
  std::list<int> l2;
  std::copy(l.begin(), l.end(), back_inserter(l2));

  // l2.erase(v.begin() + 2);       //  3番目の要素（9）を削除
  l2.erase(l2.begin());       // 先頭の要素を削除


  for(pos = l2.begin(); pos!=l2.end(); ++pos){
      cout << *pos << "\n";
  }
#endif
}

#endif
