/*

  g++ -g test20190419F-0.cpp route99.cpp bus20190419.cpp person20190419.cpp db20190419.cpp -o test20190419F-0 -llibpq -lwsock32 -lws2_32 -static-libstdc++ -I"D:\PostgreSQL\10\include" -L"D:\PostgreSQL\10\lib" 


  g++ -g test20190419F-0.cpp route99.cpp bus20190419.cpp person20190419.cpp -o test20190419F-0 -lwsock32 -lws2_32

  g++ -c test20190419F-0.cpp

  ====================================================
  ��q1�l�A�o�X1��ł̋����w�K�v���O���������������邼
  ====================================================




  F�v���O���� (�����w�K���[�h)

  �� �s���͌��\�ȒP�A����������~��5��Ԃ̍s�������Ȃ�
  �� ���͊��]�� 3�v�f
     (1)�����ȊO�̃o�X�̏ꏊ
     (2)�܂�����ɂ����q�̏ꏊ
     (3)���ݏ���Ă����q�̓����ꏊ

  �� ���[�g�v���O������9x9��route99.cpp�ɕύX

 
  B�v���O����(�n�[�E�I�[�t���̊��S�I���f�}���h)���x�[�X�Ƃ��āA�u�n�[�E�I�[�Ȃ��̊��S�I���f�}���h�v�̃v���O�������쐬����B��   �܂�direction�̊T�O������
  (�x�[�X�́A"test20190311B-0.cpp")

  �Ȍ�A"E�v���O����"�Ə̌Ă���B
  �u�悹��ׂ���q�����݂��Ȃ����́A�o�X���~������v
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
#include <list>   // list ���p�̂���
using namespace std;

#include "test20190419.h"

STATE *p_first_state, *p_last_state;  

extern list<PERSON> person_list; // �V�~�����[�^��ɑ��݂���S�Ă̐l��
extern list<BUS> bus_list; // �V�~�����[�^��ɑ��݂���S�Ẵo�X

LOCATION bus_stop[10][10];
int global_clock; // main �̒���int i�Ɠ����l�ɂ���

// �����ɂ���̂��ʓ|�Ȃ̂ŃO���[�o���ɏo���Ă���
int sd;
struct sockaddr_in addr;
WSADATA wsaData;

double d_walk_only[100][100];
double d_bus_only[100][100];
double d_walk_and_bus[100][100];

list<int> path_walk_only[100][100];   // ���������̍ŏ����ԃ��[�g
list<int> path_bus_only[100][100];   // ���������̍ŏ����ԃ��[�g
list<int> path_walk_and_bus[100][100];   // �o�X���܂߂��ŏ����ԃ��[�g

int total_home_time = 0;
int total_travel_time = 0;
int total_life_time = 0;

extern double d[100][100];  // d[i][k]�F�m�[�hi����m�[�hk�ւ̈ړ����� 
extern int via[100][100];  //  d[i][k]�̊Ԃɂ���(�����Ƃ�1��)���p�m�[�h

//// d[100][100]�̑ޔ�ϐ�


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

#define rad2deg(a) ((a)/M_PI * 180.0) /* rad �� deg �Ɋ��Z����}�N���֐� */
#define deg2rad(a) ((a)/180.0 * M_PI) /* deg �� rad �Ɋ��Z����}�N���֐� */

double distance_km(double px1, double py1, double px2, double py2, double *rad_up, int *area, int *dis)
{
  /*
	���C�����[�`���̋L�q��
	double rad_up1;
	int area;
	d_km = distance_km(px1, py1, px2, py2, &rad_up1, &area); 
  */

  double earth_r = 6378.137;

  double loRe = deg2rad(px2 - px1); // ����
  double laRe = deg2rad(py2 - py1); // ��k

  double EWD = cos(deg2rad(py1))*earth_r*loRe; // ��������
  double NSD = earth_r*laRe; //��k����

  double distance = sqrt(pow(NSD,2)+pow(EWD,2));  

  // y/x�̋t����(�A�[�N�^���W�F���g)��-��/2�`��/2�͈̔͂ŕԂ��܂��B
  *rad_up = atan2(NSD, EWD);
  
  double pi = 3.141592654;

  if ((*rad_up >= -1.0 * pi / 4.0 ) && (*rad_up < pi / 4.0 )){
	*area = 0; // 315 �` 45�x
  }
  else if ((*rad_up >= pi / 4.0 ) && (*rad_up < pi / 4.0 * 3)){
	*area = 1; // 45 �` 135�x
  }
  else if ((*rad_up <= -1.0 * pi / 4.0 ) &&  (*rad_up > -1.0 * pi / 4.0 * 3)){
	*area = 3; // 225 �` 315�x
  } 
  else {
	*area = 2; // 135�`225�x 
  }

  // "3"�̈Ӗ��� Waiting Person | Riden Person | Bus. 
  
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



//class PERSON; // PERSON�̒�`��BUS�̉��ɂ���



int make_bus_stop(void)
{
  // for (int y = -2; y < 3; y++){ // 0�`4 �� �����0�`9�ɂ���Ƃ������Ƃ́A5�𑫂��΂���(�͂�)
  for (int y = -2; y < 3 + 5; y++){ // 0�`4 + 5
	//for (int x = -4; x < 5; x++){ // 0�`8  �� �����0�`9�ɂ���Ƃ������Ƃ́A1�𑫂��΂���
	for (int x = -4; x < 5 + 1; x++){ // 0�`8 + 1
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
  // winsock2�̏�����
  WSAStartup(MAKEWORD(2,0), &wsaData);
  
  //socket�V�X�e���R�[��
  //�f�B�X�N���v�^sd���擾����
  //SOCK_DGRAM <- UDP, SOCK_STREAM<- TCP
  if((sd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
	perror("socket");
	printf("error:%d\n",WSAGetLastError());
	return -1;
  }
  
  // ���M��A�h���X�ƃ|�[�g�ԍ���ݒ肷��
  memset(&addr,0,sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(2000);   // �|�[�g�ԍ�(htons)�Ńl�b�g���[�N�o�C�g�I�[�_�[�ɕϊ�
  addr.sin_addr.s_addr = inet_addr("127.0.0.1");  //IP�A�h���X���i�[����in_addr�\����
  //addr.sin_addr.s_addr = inet_addr("49.212.198.156");  //IP�A�h���X���i�[����in_addr�\����

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
  // �ڑ���ؒf����
  closesocket(sd);
  WSACleanup();

  return 0;
}

void init_state_list(STATE **p_first_state, STATE **p_last_state)
{
  STATE *p_top_state = (STATE *)malloc(sizeof(STATE));
  if(p_top_state == NULL) {
	printf("���������m�ۂł��܂���\n");
	exit(EXIT_FAILURE);
  }
  memset(p_top_state, 0, sizeof(STATE)); // �[���N���A


  STATE *p_tail_state = (STATE *)malloc(sizeof(STATE));
  if(p_tail_state == NULL) {
	printf("���������m�ۂł��܂���\n");
	exit(EXIT_FAILURE);
  }
  memset(p_tail_state, 0, sizeof(STATE)); // �[���N���A

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
	printf("���������m�ۂł��܂���\n");
	exit(EXIT_FAILURE);
  }
  memset(new_p_state, 0, sizeof(STATE)); // �[���N���A

  return new_p_state;
}


STATE* add_state(STATE *p_ref_state)
{
  /*
	add_�̎g�����̒���

	  STATE state; // ���[�J���Ŏ��̂�����Ă�����

	  number = 123456;
	  action = rnd[1]; 
	  value = 4;
	  
	  add_state(&state); // add_�̒���malloc����
  */

  STATE *new_p_state = (STATE *)malloc(sizeof(STATE));
  if(new_p_state == NULL) {
	printf("���������m�ۂł��܂���\n");
	exit(EXIT_FAILURE);
  }
  memset(new_p_state, 0, sizeof(STATE)); // �[���N���A
  memcpy(new_p_state, p_ref_state, sizeof(STATE)); // �����̓��I�������̓��e�R�s�[
 
  write_db(new_p_state->number, new_p_state->action_type, new_p_state->q);
	

  // state�̒ǉ������L�q��������
 
  // state�̒ǉ������L�q�����܂�

  STATE *p_state = p_last_state->prev;

  p_state->next = new_p_state;
  new_p_state->prev = p_state;

  p_last_state->prev = new_p_state;
  new_p_state->next = p_last_state;

  return new_p_state;
}

int change_or_add_state(int num, int action_type, int add_q) // ��Ԕԍ������āAstate�̒l��ύX��
{
  // ===== STATE ���[�v����========
  STATE* p_state = p_first_state->next;  
    while (p_state != p_last_state){  
	  /// �������e(�J�n)

	  //�u��ԁv�ԍ��ƁA�u�s���v�ԍ�����v����ꍇ��
	  if ((p_state->number == num) && (p_state->action_type == action_type)){ 

		// Q�l�𑝂₷
		p_state->q += add_q;

		// �ړI��B������A�Ƃ��ƂƏo��
		return 1;  // ���̏ꍇ�A�Ԃ�l1 
	  }

	  /// �������e(�I��)

	  // ���̃��[�v�ɉ��
	  p_state = p_state->next;
	}

	/// ���num�͔����ł��Ȃ������̂ŁA�V�����u��ԁv�����

	STATE state; // ���[�J���Œ��g������Ă�����
	  
	state.number = num;
	state.action_type = action_type;
	state.q = add_q; // ���Z�����͂��Ȃ�
	
	add_state(&state); // add_�̒���malloc(���̉�����)

	return 2; // ���̏ꍇ�A�Ԃ�l2

}


void delete_state(STATE *p_state)  
{
  /* 
	 delete_ �̎g�����̒���

	 STATE* p_state_prev = p_state->prev;
	 delete_state(p_state);
	 p_state = p_state_prev;
  */
	 

  // �|�C���^��\��ւ���
  p_state->prev->next = p_state->next;
  p_state->next->prev = p_state->prev;
  
  // ���̃��������������
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


  

  

// 4�i���\���Ƃ݂Ȃ��āA�u��ԁv�ԍ���10�i���ŕ\��
// ���C�����[�`���ł̎g����
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
  ���Ȃ���΂Ȃ�Ȃ����̂́A�o�X�̍s���v�� �� �l�Ԃ̍s���v��
  �܂��l�����𐮗�����

  �o�X�͒��^�s��

  3.6�bx25�X�e�b�v = 90�b�ňړ�����B ����ɁA3.6�b x 5 = 18�b��Ԃ���
  1�H���Г��ʏ�^�]�� 90x8 + 18x 7 = 846�b = 14.1���̉^�s������
  �I�[��180�b  �b��Ԃ��āA�Ăщ^�s�J�n ����
  �\�\ �Ƃ���B

  ���ɏ�q�ł���B
  �l�Ԃ̕��s�͎���3.6km�Ƃ��� (�b��1m)

  �ŏ��͉^�s�J�n�O��10�l���쐬���āA���̉��H�ƋA�H��S���ԗ����郋�[�g�𑖂���̂Ƃ���

  �����̖ڕW�͂����܂�
*/ 

int make_routes()
{
  ////////// ���[�g�̎��O�쐬(���s�����̃��[�g)
  make_route_init();
  make_route_change_walk_only(); // route.cpp
  make_route_calc();
  // list<int> path_walk_only[100][100];   // �ʓ|�Ȃ̂ŁA�O���[�o���Ɏ����Ă���
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

  ////////// ���[�g�̎��O�쐬(�o�X�����̃��[�g)
  make_route_init();
  make_route_change_bus_only(); // route.cpp
  make_route_calc();
  // list<int> path_bus_only[100][100];   // �ʓ|�Ȃ̂ŁA�O���[�o���Ɏ����Ă���
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

  ////////// ���[�g�̎��O�쐬(���s�ƃo�X�̍��݃��[�g)
  make_route_init();
  make_route_change_walk_and_bus(); // �H�����e�̋����㏑��
  make_route_calc();
  
  // list<int> path_walk_and_bus[100][100];   // �ʓ|�Ȃ̂ŁA�O���[�o���Ɏ����Ă���
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

  //// q_state�e�[�u�����ǂ����邩�������Ō��߂�


  // �f�[�^�x�[�X�̑S�폜(�ŏ�������Ȃ���)
  // ��
  delete_db(); 

  // �f�[�^�x�[�X���폜�����Ȃ��œǂݍ���(�w�K���ʂ�QGIS�ȂǂłŌ��������͂�����)
  // ��
  //read_db();  
  


  //printf("pass1\n");

  make_routes();
  ////////// ���[�g�̎��O�쐬(�����܂�)

  make_bus_stop(); // �o�X��̍쐬
  open_udp(); // �ʐM������
  
  // BUS bus1("bus1",0,2);
  // BUS *bus1 = new BUS("bus1",0,2,1);
  // BUS *bus1 = new BUS("bus1",8,2,2);
  
  // �o�X�̐���
	/////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////
  for(int i = 1; i <= 1; i++){ // �o�X�̑䐔�͂����Œ���
	//////////////////////////////////////2///////////////////////
	/////////////////////////////////////////////////////////////
	if ( i == 1){
	  char name[10] ={0};
	  sprintf(name,"bus%d", i);
	  //BUS *bus = new BUS(name,0,2,1);
	  BUS *bus = new BUS(name,2,2,1);
	  bus->init_state_seq_list();
	  
	  // ���X�g�̒ǉ�
	  bus_list.push_back(*bus);

	}
	else if (i == 2){
	  char name[10] ={0};
	  sprintf(name,"bus%d", i);
	  BUS *bus = new BUS(name,8,2,2);
	  bus->init_state_seq_list();

	  // ���X�g�̒ǉ�
	  bus_list.push_back(*bus);
	}
	if ( i == 3){
	  char name[10] ={0};
	  sprintf(name,"bus%d", i);
	  BUS *bus = new BUS(name,4,2,3);
	  bus->init_state_seq_list();
	  
	  // ���X�g�̒ǉ�
	  bus_list.push_back(*bus);

	}
	if ( i == 4){
	  char name[10] ={0};
	  sprintf(name,"bus%d", i);
	  BUS *bus = new BUS(name,4,2,4);
	  bus->init_state_seq_list();
	  
	  // ���X�g�̒ǉ�
	  bus_list.push_back(*bus);
	}
  }

  // ��ԃ��X�g�̏�����(�o�X���ɍ����)
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
	// �v���p�ϐ�
	static int bus_status_count_0 = 0; // 0:��~�� 
	static int bus_status_count_1 = 1; // 1:�^�s�� 
	////////////////////////////
	
	////////////////////////////
	// �v���p�ϐ�
	static int person_status_count_0 = 0; // 0:�o���O 
	static int person_status_count_1 = 0; // 1:��~�� 
	static int person_status_count_2 = 0; // 2:���s��
	static int person_status_count_3 = 0; // 3:��Ԓ�
	static int person5_status_count_2 = 0; // 2:���s��
	////////////////////////////

	clean_qgis_display(); // QGIS��ʂ̃N���A

	person_list_flag = 0;
	person_list.clear(); // person_list�̍폜

	int i = 0;
	
	/////////////////////////////////////////////////////////////
	//for(int i = 0; i < 10000; i++){
	for(int i = 0; i < 100000; i++){
	//while(1){
	  /////////////////////////////////////////////////////////////
	  
	  printf("%d \n",i);
	  global_clock = i; // �O���[�o���N���b�N�́A�O���[�o���ϐ�
	  
	  // �u�Ȃ�ł��T�u���[�`���v�͂�߂悤
	  // ���C�����[�`���Ŏ�𔲂���(�ǂ����ė��p�Ȃ񂩂��Ȃ�����)
	  
	  //	if (person_num < 10){ // �ő�10�l���
	  /////////////////////////////////////////////////////////////
	  /////////////////////////////////////////////////////////////
	  
	  if (person_num <= 1){ // 9�l���
		//if (rand() % 25 == 0){
		if (rand() % 1 == 0){
		  
		  //if (person_num <= 100){ // 100�l���
		  //if (rand() % 100000 == 0){
		  
		  ////////////////////////////////////////////////////////////
		  /////////////////////////////////////////////////////////////
		  // 25���[�v��1�񂭂炢�̊m���ŗ��p�҂𔭐������邱�Ƃɂ���
		  // ���p�҂̍s������e�L�g�[�Ɍ��߂�
		  int dep_grid_x;
		  int dep_grid_y;
		  int arr_grid_x;
		  int arr_grid_y;
		  
		  do {
			//dep_grid_x = rand() % 9; // 0�`8�܂ł̒l���o�Ă���
			dep_grid_x = rand() % 10; // 0�`9�܂ł̒l���o�Ă���
			
			//dep_grid_y = rand() % 5; // 0�`4�܂ł̒l���o�Ă���
			dep_grid_y = rand() % 10; // 0�`9�܂ł̒l���o�Ă���
			
			// arr_grid_x = rand() % 9; // 0�`8�܂ł̒l���o�Ă���
			arr_grid_x = rand() % 10; // 0�`9�܂ł̒l���o�Ă���
			
			//arr_grid_y = rand() % 5; // 0�`4�܂ł̒l���o�Ă���
			arr_grid_y = rand() % 10; // 0�`9�܂ł̒l���o�Ă���
			
			// 2��grid�͓����ꏊ�ł����Ă͂Ȃ�Ȃ�(���R)
			//��
			// 2��grid�̋����́A500���[�g���ȏ�(����750���[�g���ȏ㗣��Ă��邱�ƂƂ���
			//} while((dep_grid_x == arr_grid_x) && (dep_grid_y == arr_grid_y))
		  } while (( abs(dep_grid_x - arr_grid_x) + abs(dep_grid_y - arr_grid_y)) <= 3 );
		  
		  char name[10] ={0};
		  sprintf(name,"person%02d", person_num);
		  
		  cout << "i =" << i << "\n";
		  cout << "person_num =" << person_num << " " << name << "\n";
		  cout << "x:" << dep_grid_x << " y:" << dep_grid_y << " x:" 
			   << arr_grid_x << " y:" << arr_grid_y << "\n";
		  
		  // ��q�̐���
		  PERSON *person 
			= new PERSON(person_num, name, dep_grid_x, dep_grid_y, arr_grid_x, arr_grid_y);
		  
		  // person->send_udp();
		  
		  // ���X�g�ւ̒ǉ�
		  person_list.push_back(*person);
		  // delete person; // �폜���Ă����̂��H
		  
		  person_num += 1;
		}
	  }
	  
	  /////////////////////////////////////////////////////////////////////
	  

#if 1

	  if ((speed_down_count % 5 == 0) && (speed_down_count != 0)){
		  

		struct timespec ts;
		ts.tv_sec = 0;
		ts.tv_nsec = 999999999/999; // 1/350�b�ƌ��Ȃ��ėǂ�
		nanosleep(&ts, NULL);
		

	  }
		

#endif

	  //usleep(100000/35); // 0.2�b
	  //usleep(100000/5); // 0.2�b
	  
	  
	  //usleep(100000); // 0.2�b
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
	  // �ϑ��ƋL�^�𓊓�
	  for(b_pos = bus_list.begin(); b_pos!=bus_list.end(); ++b_pos){
		b_pos->check_environment();
		b_pos->add_state_seq();
	  }
	  
#endif 
  	  
	  // �v����p���[�`��
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
	  
	  // ������q�����X�g����O��(���A���̑O�ɕK�v�ȏ������o��)
	  pos = person_list.begin();
	  while( pos != person_list.end()) {
		if (pos->person_status == -1){
		  person_list_flag = 1; //�t���O���グ��
		  
		  cout << "Erased person num:" << pos->number <<"\n";
		  cout << "home_time:" << pos->home_time <<"\n";
		  
		  if (pos->arr_time == -1){ // �����������L�^
			pos->arr_time = global_clock;
			pos->travel_time = pos->arr_time - pos->dep_time;
		  }
		  cout << "travel_time:" << pos->travel_time <<"\n";
		  
		  if (pos->life_time == -1){ // �����������L�^
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
	  
	  // �I������
	  if ((person_list_flag == 1) && (person_list.size() == 0)){
		printf("bus_status_count_0 = %d\n",bus_status_count_0);
		printf("bus_status_count_1 = %d\n",bus_status_count_1);
		printf("person_status_count_0 = %d\n",person_status_count_0); // �ꎞ��~
		printf("person_status_count_1 = %d\n",person_status_count_1); // 1:���s����
		printf("person_status_count_2 = %d\n",person_status_count_2); // 2:�o�X�҂�����
		printf("person_status_count_3 = %d\n",person_status_count_3); // 3:�o�X��Ԏ���
		
		printf("total_home_time = %d\n",total_home_time);
		printf("total_travel_time = %d\n",total_travel_time);
		printf("total_life_time = %d\n",total_life_time);
		
		
		for(b_pos = bus_list.begin(); b_pos!=bus_list.end(); ++b_pos){
		  if ((100000 - i) > 1){
			b_pos->q_reward(100000 - i);
			
			speed_down_count += 1;		
			printf("q_reward speed_down_count %d\n ", speed_down_count);

			//b_pos->q_reward(10*(10000 - i));
			b_pos->del_state_seq(); // seq���X�g���폜
		  }
		}
	
		show_state();
		person_list_flag = 0;		
		
		finish_flag = 1;

		//exit(0);
	  }	// �I������ if ((person_list_flag == 1) && (person_list.size() == 0))

	  if (finish_flag == 1){
		finish_flag = 0;
		break;
	  }
	}
	
	for(b_pos = bus_list.begin(); b_pos!=bus_list.end(); ++b_pos){
	  b_pos->del_state_seq(); // seq���X�g���폜
	}

  }  // while(1)

  close_udp();
  close_db();

}
