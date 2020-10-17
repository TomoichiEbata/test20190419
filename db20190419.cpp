/*


  g++ -c db20190419.cpp

  g++ -g db20190419.cpp -o db20190419 -I"D:\PostgreSQL\10\include" -L"D:\PostgreSQL\10\lib" -llibpq -lwsock32 -lws2_32


  // 状態をPostgreSQLに書き込み続ける
  // プログラム実行中にも、経過情報が読めるようにする為


  まず、以下の手順でテーブルを作っておく
 
  C:\Users\yrl-user>psql -h localhost -U postgres	
  postgres=# \l		
  postgres=# \connect ca_db(データベース名)
  
  以下をコピペしてテーブルとその内容を直接書き込む
  
  create table q_state (
	   number int,
	   action_type int,
	   q int);

ca_db=# \dt
 
●データベースのテーブルの中身を確認する
ca_db=# select * from users; // 検索の基本形
 
*/

#include <stdio.h>
#include <stdlib.h>
//#include <unistd.h> // sleep(1)
#include <time.h> // sleep(1)
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include "libpq-fe.h"

#include <list>   // list 利用のため
using namespace std;

#include "test20190419.h"

extern STATE* add_state(STATE *p_ref_state);

const char *conninfo = "host=localhost user=postgres password=c-anemone dbname=ca_db port=5433";
PGconn *conn;

int open_db()
{
  // データベースとの接続を確立する 
  //PGconn *conn = PQconnectdb(conninfo);
  conn = PQconnectdb(conninfo);
  
  /* バックエンドとの接続確立に成功したかを確認する */
  if (PQstatus(conn) != CONNECTION_OK){
	fprintf(stderr, "Connection to database failed: %s",
			PQerrorMessage(conn));
  }
  
  return 0;
};

int delete_db()
{
  // テーブルの中身を空にする
  char stringSQL2[512] = {0};
  sprintf(stringSQL2, "DELETE FROM q_state;");

  PGresult *res = PQexec(conn, stringSQL2);

  // INSERT等値を返さないコマンドの場合戻り値は PGRES_COMMAND_OK
  if (res == NULL || PQresultStatus(res) != PGRES_COMMAND_OK) {
    // SQLコマンドが失敗した場合
	fprintf(stderr, "INSERT COMMAND IS FAILED.",
			PQerrorMessage(conn));
  }
  
  // 値セットが無い場合でも必ず結果をクリアする
  PQclear(res);

  return 0;
};

int read_db()
{
  // テーブルオープン 
  PGresult *res = PQexec(conn, "SELECT * from q_state;");
  if (res == NULL){
	fprintf(stderr, "[SELECT name from q_state;] failed: %s",
			PQerrorMessage(conn));
  }
  
  // SELECTの場合戻り値は PGRES_TUPLES_OK.  これを確認しておく
  if (PQresultStatus(res) != PGRES_TUPLES_OK){
	PQerrorMessage(conn);
  }
  
  // まず属性名を表示する。 
  int nFields = PQnfields(res);
  for (int i = 0; i < nFields; i++){
	printf("%-15s", PQfname(res, i));
  }
  
  // そして行を表示する。
  for (int i = 0; i < PQntuples(res); i++) {
	
	STATE state;

	state.number = atoi(PQgetvalue(res, i, 0));
	state.action_type = atoi(PQgetvalue(res, i, 1));
	state.q = atoi(PQgetvalue(res, i, 2));

	add_state(&state);

  }
 
  // PQexecを使用した場合、不要になった時点で結果セットをクリア
  PQclear(res);

  return 0;
  
}
  
int write_db(int number, int action_type, int q)
{
  char stringSQL2[512] = {0};
  char stringSQL1[512] = {0};
  
  sprintf(stringSQL2, "UPDATE q_state SET number = %d, action_type = %d, q = %d where number = %d and action_type = %d;",
		  number, action_type, q, number, action_type);

  PGresult *res = PQexec(conn, stringSQL2);
  ExecStatusType est = PQresultStatus(res);
  
  char mes[10] = {0};
  strcpy(mes,PQcmdStatus(res));
  // UPDATEできなかった場合には、"UPDATE 0"というメッセージが帰ってくるので、
  // そのメッセージを使って、UPDATEの失敗を判断して、INSERTの処理を行う
  int c = strcmp(mes, "UPDATE 0"); 
  if (c == 0){
	sprintf(stringSQL1, "INSERT INTO q_state(number , action_type , q) VALUES(%d,%d,%d);",
			number, action_type, q);

	PGresult *res = PQexec(conn, stringSQL1);
  }
  
  // INSERT等値を返さないコマンドの場合戻り値は PGRES_COMMAND_OK
  if (res == NULL || PQresultStatus(res) != PGRES_COMMAND_OK) {
    // SQLコマンドが失敗した場合
	fprintf(stderr, "INSERT COMMAND IS FAILED.",
			PQerrorMessage(conn));
  }
  
  // 値セットが無い場合でも必ず結果をクリアする
  PQclear(res);

  return 0;
};

int close_db()
{
  // 最後必ずデータベースとの接続を閉じる
  PQfinish(conn);

  return 0;
}

#if 0

int main()
{
  open_db();
  delete_db();

  write_db(1, 2, 4);
  write_db(1, 2, 3);
  write_db(2, 1, 4);
  write_db(2, 2, 4);
  write_db(2, 1, 5);
  write_db(1, 2, 5);

  read_db();

  
  struct timespec ts;
  fprintf(stderr, "start\n");
  ts.tv_sec = 0;
  ts.tv_nsec = 999999999/35; // 1/35秒と見なして良い
  nanosleep(&ts, NULL);
  fprintf(stderr, "end\n");
  

  close_db();

}


#endif //1
