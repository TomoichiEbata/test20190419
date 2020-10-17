/*


  g++ -c db20190419.cpp

  g++ -g db20190419.cpp -o db20190419 -I"D:\PostgreSQL\10\include" -L"D:\PostgreSQL\10\lib" -llibpq -lwsock32 -lws2_32


  // ��Ԃ�PostgreSQL�ɏ������ݑ�����
  // �v���O�������s���ɂ��A�o�ߏ�񂪓ǂ߂�悤�ɂ����


  �܂��A�ȉ��̎菇�Ńe�[�u��������Ă���
 
  C:\Users\yrl-user>psql -h localhost -U postgres	
  postgres=# \l		
  postgres=# \connect ca_db(�f�[�^�x�[�X��)
  
  �ȉ����R�s�y���ăe�[�u���Ƃ��̓��e�𒼐ڏ�������
  
  create table q_state (
	   number int,
	   action_type int,
	   q int);

ca_db=# \dt
 
���f�[�^�x�[�X�̃e�[�u���̒��g���m�F����
ca_db=# select * from users; // �����̊�{�`
 
*/

#include <stdio.h>
#include <stdlib.h>
//#include <unistd.h> // sleep(1)
#include <time.h> // sleep(1)
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include "libpq-fe.h"

#include <list>   // list ���p�̂���
using namespace std;

#include "test20190419.h"

extern STATE* add_state(STATE *p_ref_state);

const char *conninfo = "host=localhost user=postgres password=c-anemone dbname=ca_db port=5433";
PGconn *conn;

int open_db()
{
  // �f�[�^�x�[�X�Ƃ̐ڑ����m������ 
  //PGconn *conn = PQconnectdb(conninfo);
  conn = PQconnectdb(conninfo);
  
  /* �o�b�N�G���h�Ƃ̐ڑ��m���ɐ������������m�F���� */
  if (PQstatus(conn) != CONNECTION_OK){
	fprintf(stderr, "Connection to database failed: %s",
			PQerrorMessage(conn));
  }
  
  return 0;
};

int delete_db()
{
  // �e�[�u���̒��g����ɂ���
  char stringSQL2[512] = {0};
  sprintf(stringSQL2, "DELETE FROM q_state;");

  PGresult *res = PQexec(conn, stringSQL2);

  // INSERT���l��Ԃ��Ȃ��R�}���h�̏ꍇ�߂�l�� PGRES_COMMAND_OK
  if (res == NULL || PQresultStatus(res) != PGRES_COMMAND_OK) {
    // SQL�R�}���h�����s�����ꍇ
	fprintf(stderr, "INSERT COMMAND IS FAILED.",
			PQerrorMessage(conn));
  }
  
  // �l�Z�b�g�������ꍇ�ł��K�����ʂ��N���A����
  PQclear(res);

  return 0;
};

int read_db()
{
  // �e�[�u���I�[�v�� 
  PGresult *res = PQexec(conn, "SELECT * from q_state;");
  if (res == NULL){
	fprintf(stderr, "[SELECT name from q_state;] failed: %s",
			PQerrorMessage(conn));
  }
  
  // SELECT�̏ꍇ�߂�l�� PGRES_TUPLES_OK.  ������m�F���Ă���
  if (PQresultStatus(res) != PGRES_TUPLES_OK){
	PQerrorMessage(conn);
  }
  
  // �܂���������\������B 
  int nFields = PQnfields(res);
  for (int i = 0; i < nFields; i++){
	printf("%-15s", PQfname(res, i));
  }
  
  // �����čs��\������B
  for (int i = 0; i < PQntuples(res); i++) {
	
	STATE state;

	state.number = atoi(PQgetvalue(res, i, 0));
	state.action_type = atoi(PQgetvalue(res, i, 1));
	state.q = atoi(PQgetvalue(res, i, 2));

	add_state(&state);

  }
 
  // PQexec���g�p�����ꍇ�A�s�v�ɂȂ������_�Ō��ʃZ�b�g���N���A
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
  // UPDATE�ł��Ȃ������ꍇ�ɂ́A"UPDATE 0"�Ƃ������b�Z�[�W���A���Ă���̂ŁA
  // ���̃��b�Z�[�W���g���āAUPDATE�̎��s�𔻒f���āAINSERT�̏������s��
  int c = strcmp(mes, "UPDATE 0"); 
  if (c == 0){
	sprintf(stringSQL1, "INSERT INTO q_state(number , action_type , q) VALUES(%d,%d,%d);",
			number, action_type, q);

	PGresult *res = PQexec(conn, stringSQL1);
  }
  
  // INSERT���l��Ԃ��Ȃ��R�}���h�̏ꍇ�߂�l�� PGRES_COMMAND_OK
  if (res == NULL || PQresultStatus(res) != PGRES_COMMAND_OK) {
    // SQL�R�}���h�����s�����ꍇ
	fprintf(stderr, "INSERT COMMAND IS FAILED.",
			PQerrorMessage(conn));
  }
  
  // �l�Z�b�g�������ꍇ�ł��K�����ʂ��N���A����
  PQclear(res);

  return 0;
};

int close_db()
{
  // �Ō�K���f�[�^�x�[�X�Ƃ̐ڑ������
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
  ts.tv_nsec = 999999999/35; // 1/35�b�ƌ��Ȃ��ėǂ�
  nanosleep(&ts, NULL);
  fprintf(stderr, "end\n");
  

  close_db();

}


#endif //1
