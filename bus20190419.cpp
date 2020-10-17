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
#include <list>   // list ���p�̂���
using namespace std;

#include "test20190419.h"

extern STATE *p_first_state, *p_last_state;  


extern double distance_km(double px1, double py1, double px2, double py2, double *rad_up, int *area, int *dis);

extern STATE* make_state(int num, int action, int q);
extern int make_number(int* field_check);
extern STATE* add_state(STATE *p_ref_state);

extern list<PERSON> person_list; // �V�~�����[�^��ɑ��݂���S�Ă̐l��(main�ɂ��܂�)
list<BUS> bus_list; // �V�~�����[�^��ɑ��݂���S�Ẵo�X

extern int sd;
extern struct sockaddr_in addr;
extern WSADATA wsaData;

extern LOCATION bus_stop[10][10];
extern double d_bus_only[100][100];
extern double d_walk_only[100][100];
extern double d_walk_and_bus[100][100];

extern list<int> path_bus_only[100][100];   // ���������̍ŏ����ԃ��[�g


RND rnd[] = {{ 0, 0},  
			 { 1, 0},  
			 { 0, 1},  
			 {-1, 0},  
			 { 0,-1}}; 


BUS::BUS(const char *n, int prev_grid_gx, int prev_grid_gy, int route_case){

  number = route_case; // �P�[�X���o�X�̔ԍ��ɂ���("-1"�̒l�͂��肦�Ȃ�)
  
  strcpy(name, n);
  
  stop_timer = 5; // �o�X�◯���̃X�e�b�v��
  bus_status = 1; // �o�X�^�s:1 �o�X���:0
  
  prev_grid.gx = next_grid.gx = prev_grid_gx;
  prev_grid.gy = next_grid.gy = prev_grid_gy;
  
  present.px = prev_bus_stop.px = next_bus_stop.px 
	= bus_stop[prev_grid_gx][prev_grid_gy].px;
  present.py = prev_bus_stop.py = next_bus_stop.py 
	= bus_stop[prev_grid_gx][prev_grid_gy].py;
  
  ////// �o�X�̉������[�g�̍쐬
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
  // �u�w�����Ȃ��v�ƒ�`����
  direction = -1;  
  
  
  // �Ƃ܂��A�\���傫�Ȑ�������Ă����΁A�ǂ����ȁA�ƁA
  
  ////// �o�X�̉������[�g�̍쐬(�I���)
  
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
  
  //  printf("%s",ch); // �m�F�p
  
  if (sendto(sd, ch, strlen(ch), 0, (struct sockaddr *)&addr,  sizeof(addr)) < 0) {        
	perror("sendto");
	printf("error:%d\n",WSAGetLastError());
	return -1;
  }
  return 0;
}

int BUS::bus_change_direction(int next_grid_gx, int next_grid_gy){ // ���̌o�H������

  printf("1 ::bus_change_direction:prev.....%d (%d,%d)\n",number, prev_grid.gx, prev_grid.gy);  
  printf("1 ::bus_change_direction:next.....%d (%d,%d)\n",number, next_grid.gx, next_grid.gy);  


  // �̈搧��
  if (next_grid_gx > 9) 
	next_grid_gx = 9;
  else if (next_grid_gx < 0) 
	next_grid_gx = 0;

  if (next_grid_gy > 9) 
	next_grid_gy = 9;
  else if (next_grid_gy < 0) 
	next_grid_gy = 0;
  // �̈搧��(�����܂�)
  
  prev_grid.gx = next_grid.gx;
  prev_grid.gy = next_grid.gy;
  
  next_grid.gx = next_grid_gx;
  next_grid.gy = next_grid_gy;
  
  prev_bus_stop.px = next_bus_stop.px;
  prev_bus_stop.py = next_bus_stop.py;	
  next_bus_stop.px = bus_stop[next_grid.gx][next_grid.gy].px;
  next_bus_stop.py = bus_stop[next_grid.gx][next_grid.gy].py;
  
  // �E�[�◯��(82)(�ɓ������Ă�̂ł���΁A����(2)�Ƃ���B
  // ���[�◯��(02)(�ɓ������Ă�̂ł���΁A���(1)�Ƃ���B

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


  // �܂���ԕϐ������Z�b�g����
  memset(field_check, 0, sizeof(field_check));

  // �����̏悹�Ă����q�̍~�Ԉʒu�͂ǂ����H
  list<PERSON>::iterator bp;
  for(bp = passenger.begin(); bp!=passenger.end(); ++bp){	// �o�X�ɏ���Ă����q
	if (bp->person_status == 3){ // 3:��Ԓ�
	  // _arr_xx �͈Ӗ����ς��Ă��邪�A�ύX����Ɩʓ|�ɂȂ�̂ł��̂܂�
	  
	  double d_km = distance_km(present.px, present.py, 
								bp->arrival.px, bp->arrival.py, 
								&rad_up1, &area, &dis); 

	  if (dis > field_check[0][area]){
		field_check[0][area] = dis;
	  }
	}
  }

  // �o�X��҂��Ă����q�̈ʒu�͂ǂ����H 

  list<PERSON>::iterator pos;
  for(pos = person_list.begin(); pos!=person_list.end(); ++pos){
	// �o�X��Ԓ�(3)�ł͂Ȃ�
	if (pos->person_status != 3){
	  // ����(-1)�����Ă��Ȃ�
	  if (pos->person_status != -1){ 
		// ���s��(0)�ł��Ȃ�
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

  // �����ȊO�̃o�X�̈ʒu�͂ǂ����H

  //list<BUS>::iterator b_pos;
  for(b_pos = bus_list.begin(); b_pos!=bus_list.end(); ++b_pos){	
	if (b_pos->number != number){ // �����ȊO�̃o�X
	  
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

  //// ��Ԕԍ��̕t�^ ��Ԃ�4�i���Ɍ����ĂāA�����10�i���̐����ɕϊ�����
  //// ����4^12=16777216 �� 5�A�N�V�����K�v�ɂȂ邩��A 83886080
  //// int�^�͈̔͂��A	               -2147483648 �` 2147483647
  //// �ƂȂ�̂ŁA��Ԃ�int�ŕ\���\�ƂȂ�n�Y
  //// �\���� Q[status_number][action_number] �ŗǂ��Ǝv��

  printf("before make_number\n");
  int number = make_number((int *)field_check);
  printf("after make_number\n");

  return number;
}

int BUS::driver_q()
{
  if (bus_status == 1){ // �o�X�������Ă���
	
	// �܂��A1�X�e�b�v��(3.6�b)��i�߂�
	// const double diff_x = 0.00276; // X������250���[�g��
	// const double diff_y = 0.00225; // Y������250���[�g��
	// 1�X�e�b�v��10m �i�� �� ����10km ������A��̒l��"25"�Ŋ���Ηǂ�
	// �� (10.0/250.0)����Z����
	
	double rad = atan2((next_bus_stop.py - present.py), 
					   (next_bus_stop.px - present.px));
	// present���X�V����
	present.py += diff_y * (10.0/250.0) * sin(rad);
	present.px += diff_x * (10.0/250.0) * cos(rad);
	
	double rad_0 = atan2((next_bus_stop.py - prev_bus_stop.py),
						 (next_bus_stop.px - prev_bus_stop.px));
	
	double rad_1 = atan2((next_bus_stop.py - present.py), 
						 (next_bus_stop.px - present.px));

	// �◯�����z���� // ���S���s�̏ꍇ�ɔ��f���ł��Ȃ��̂ŁA�኱�̌덷������
	if (fabs(rad_0 -rad_1) >= pi * 0.5 - 0.0001){ 
	
	  bus_status = 0; // �o�X���Ԃ�����
	  
	  // �o�X��Ɉʒu���Œ肷��
	  present.py = next_bus_stop.py;
	  present.px = next_bus_stop.px;
	}
  }

  else if (bus_status == 0){ // �o�X����Ԃ��Ă���
	stop_timer -= 1; // �X�e�b�v���Ԃ�1���Z
	
	if ( stop_timer < 0){ // �X�e�b�v���Ԃ��g���ʂ���

	  //printf("\n bus:bus stop\n");
	  
	  bus_status = 1; // �o�X�𓮂���
	  stop_timer = 5; // �X�e�b�v����߂�
	  
	  int num = check_environment(); // �����`�F�b�N����

	  // �����܂ł́Adriver()�Ɠ���

	  // �V������Ԃ��o�ꂵ����A�A�N�V����(5��)����S������đҋ@����A�Ƃ������@�����

	  // ��Ԃ̒��ɁA�A�N�V���������q�ɂ���Ƃ������Ƃ��l�������A��������ƁAQ�w�K��
	  // �ߋ��ɑk�鎞�A��Ԃ��L�����Ȃ���΂Ȃ�Ȃ��Ƃ����̂��ʓ|�Ȃ̂ŁA�A�N�V�������Ƃ�
	  // ��Ԃ���邱�Ƃɂ���
	  
	  // ��ԋ�Ԃ�5�{�ɂȂ�̂͐h���̂ŁA��Ԃ����������i�K�ŏ�Ԃ�ǉ�����������@�����
	  
	  // �܂����݂̊�����ԂƂ��đ��݂��邩�ۂ����m�F����
	  
	  // number�ŏ�Ԃ�T��
	  STATE* p_state = p_first_state->next;  
	  STATE* q_state = NULL;
	  
	  int q_max = -1;
	  while (p_state != p_last_state){  
		/// �������e(�J�n)
		//�u��ԁv�ԍ�����v����ꍇ��
		if (p_state->number == num){
		  q_state = p_state;
		  break;
		}
		p_state = p_state->next;
	  }
	
	  if (q_state == NULL){  // ��Ԕԍ��͌������Ȃ�����
		for (int i = 0; i < 5; i ++){  // �V�݂���

		  STATE st;
		  st.number = num;
		  st.action_type = i; 
		  st.q = rand() % 10 + 1; // �Ƃ肠�����A�����l��1�`10�Ƃ������Ƃ�
		  //st.q = rand() % 100; // �Ƃ肠�����A�����l��0�`99�Ƃ������Ƃ�
		  
		  STATE* s = add_state(&st);
		  
		  if (i == 0){
			q_state = s;
		  }
		
		 
		}
	  }

	  // ����ŏ�Ԃ͑S�����݂��Ă���n�Y�ł���B(�������Ԃ����ʂł͂��邪)

	  if ((double)rand()/RAND_MAX < 0.3){  // ��:0.3�ȉ�
	  //if ((double)rand()/RAND_MAX < 0.6){  // ��:0.3�ȉ�
	  //if ((double)rand()/RAND_MAX < 0.9){  // ��:0.3�ȉ�
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
		
		for (int i = 0 ; i < d ; i++){ // q_state�́Aaction �� 0�̂��̂�����
		  q_state = q_state->next;
		}
		add_state_seq(q_state);  
		return 0;
	  }
	  else{  // ��:0.3�ȏ�
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

		for (int i = 0 ; i < a ; i++){ // q_state�́Aaction �� 0�̂��̂�����
		  q_state = q_state->next;
		}
		add_state_seq(q_state); // q_state�́Aaction �� 0�̂��̂�����

		return 0;
	  }
	}
  }

  return -1; // �����ɂ�����G���[
}

int BUS::driver()
{
  if (bus_status == 1){ // �o�X�������Ă���
	
	// �܂��A1�X�e�b�v��(3.6�b)��i�߂�
	// const double diff_x = 0.00276; // X������250���[�g��
	// const double diff_y = 0.00225; // Y������250���[�g��
	// 1�X�e�b�v��10m �i�� �� ����10km ������A��̒l��"25"�Ŋ���Ηǂ�
	// �� (10.0/250.0)����Z����
	
	double rad = atan2((next_bus_stop.py - present.py), 
					   (next_bus_stop.px - present.px));
	// present���X�V����
	present.py += diff_y * (10.0/250.0) * sin(rad);
	present.px += diff_x * (10.0/250.0) * cos(rad);
	
	double rad_0 = atan2((next_bus_stop.py - prev_bus_stop.py),
						 (next_bus_stop.px - prev_bus_stop.px));
	
	double rad_1 = atan2((next_bus_stop.py - present.py), 
						 (next_bus_stop.px - present.px));

	// �◯�����z���� // ���S���s�̏ꍇ�ɔ��f���ł��Ȃ��̂ŁA�኱�̌덷������
	if (fabs(rad_0 -rad_1) >= pi * 0.5 - 0.0001){ 
	
	  bus_status = 0; // �o�X���Ԃ�����
	  
	  // �o�X��Ɉʒu���Œ肷��
	  present.py = next_bus_stop.py;
	  present.px = next_bus_stop.px;
	}
  }

  else if (bus_status == 0){ // �o�X����Ԃ��Ă���
	stop_timer -= 1; // �X�e�b�v���Ԃ�1���Z
	
	if ( stop_timer < 0){ // �X�e�b�v���Ԃ��g���ʂ���

	  //printf("\n bus:bus stop\n");
	  
	  bus_status = 1; // �o�X�𓮂���
	  stop_timer = 5; // �X�e�b�v����߂�

#if 1
	  ///// 2019/04/02 �ǋL ///////
	  // �ҋq�[���A��q�[���̏ꍇ�́A���X�g���폜���āA�o�X��������~������
	  // ���ʂɑ����Ă���ܑ͖̂̂Ȃ��̂�
	  
	  // �t���O 
	  int d_flag1 = 0; // �ҋq�[��
	  int d_flag2 = 0; // ��q�[��
	  
	  list<PERSON>::iterator pos;
	  for(pos = person_list.begin(); pos!=person_list.end(); ++pos){
		// �o�X��Ԓ�(3)�ł͂Ȃ�
		if (pos->person_status != 3){
		  // ����(-1)�����Ă��Ȃ�
		  if (pos->person_status != -1){ 
			// ���s��(0)�ł��Ȃ�
			if (pos->person_status != 0){ 
			  d_flag1 = 1;  // �o�X�҂��̏�q����
			}
		  }
		}
	  }
	  
	  if (passenger.empty() == 0){// ��q�[���ł͂Ȃ�
		d_flag2 = 1; // ��q����
	  }
	  
	  if ((d_flag1 + d_flag2) == 0){ 
		p_bus_only_path.clear(); // ���[�g���폜����
		
		bus_status = 0; // �o�X�𓮂����Ȃ�		
		
		// printf("name:%s, ***pass**\n",name);

		return 0;
	  }

	  
#endif //1

	  
	  // ���]����J�n
	  //if ((next_grid.gx == 8) && (next_grid.gy == 2)){
	  //direction = 2;
	  //printf("direction = 2\n");
	  //}
	  //else if ((next_grid.gx == 0) && (next_grid.gy == 2)){
	  //	direction = 1;		
	  //	printf("direction = 1\n");
	  //}
	  
	  // ���ӏ󋵂�c�����J�n����
	  // �t�B�[���h�ɑ��݂��Ă���A�o�X��ҋ@���Ă���S�Ă̏�q�̏��(�ʒu�ƃX�e�[�^�X)
	  // �𒲂ׂ�
	  
	  // �����̌��݈ʒu(���W)�́Anext_grid.gx, next_grid.gy
	  // �i�s�����́A����1 ����2
	  
	  shortest = 999.9;
	  bus_flag = -1;
	  
	  _temp_enforced_pickup_person_number = -1; // (��)��q�ԍ��̑ޔ�
	  _temp_enforced_pickup_gx = -1; // (��)������ԃ|�C���g(gx)
	  _temp_enforced_pickup_gy = -1; // (��)������ԃ|�C���g(gx)
	  
	  _temp_enforced_dropoff_person_number = -1; // (��)��q�ԍ��̑ޔ�
	  _temp_enforced_dropoff_gx = -1; // (��)������ԃ|�C���g(gx)
	  _temp_enforced_dropoff_gy = -1; // (��)������ԃ|�C���g(gx)
	  
	  // if (direction == 1){ 	  // �i�s�����́A����1
	  if (direction == -1){ 	  // �i�s�����́u���w��:-1�v
		
		//��Βʉߓ_(����������q�͕K���E���́A���̌����ɂ���)
		
		// �I�[���W��(8,2)
		//shortest = d_bus_only[next_grid.gx * 10 + next_grid.gy][ 8 * 10 + 2]; 
		// dropoff��D�悷��ׁA�ŏI���W�v�Z�́A(���ӓI��)�኱���Ԃ𒷂�����
		//shortest += 0.025 / 10.0;
		
		//n_gx = 8;
		//n_gy = 2;
		//bus_flag = 1;
		
		
		/////////////////////////
		// dropoff�|�C���g���v�Z����
		// (dropoff��pickup���D�悷��(shortest���s����))
		/////////////////////////
		
		list<PERSON>::iterator bp;
		for(bp = passenger.begin(); bp!=passenger.end(); ++bp){	// �o�X�ɏ���Ă����q
		  if (bp->person_status == 3){ // 3:��Ԓ�
			// _arr_xx �͈Ӗ����ς��Ă��邪�A�ύX����Ɩʓ|�ɂȂ�̂ł��̂܂�
			
			for (int i = 0; i < 1; i++){
			  _arr_gx = bp->arr_grid.gx + rnd[i].x; 
			  _arr_gy = bp->arr_grid.gy + rnd[i].y;
			  
			  // ���E���`�F�b�N���� ( (0 <= _arr_gx <= 8) and (0 <= _arr_gy <= 4))
			  //if ((0 <= _arr_gx) && (_arr_gx <= 8) && (0 <= _arr_gy) && (_arr_gy <= 4)){
			  if ((0 <= _arr_gx) && (_arr_gx <= 9) && (0 <= _arr_gy) && (_arr_gy <= 9)){
				//�������O���������Ȃ�(�Ƃ����͎̂~�߂�)
				//if (_arr_gx >=next_grid.gx){ 
				if (shortest > d_bus_only[next_grid.gx * 10 + next_grid.gy][_arr_gx * 10 + _arr_gy]){
				  shortest = d_bus_only[next_grid.gx * 10 + next_grid.gy][_arr_gx * 10 + _arr_gy];
				  n_gx = _arr_gx;
				  n_gy = _arr_gy;
				  bus_flag = 1;
				  
				  // �m�肵����o�X����A�w��̏�q�ɁA�~�Ԓn�_��������
				  // �w�肳�ꂽ��q�́A���̒n�_���Adropoff�Ƃ��Ď����œo�^����
				  _temp_enforced_dropoff_person_number = bp->number; // (��)��q�ԍ��̑ޔ�
				  _temp_enforced_dropoff_gx = n_gx; // (��)������ԃ|�C���g(gx)
				  _temp_enforced_dropoff_gy = n_gy; // (��)������ԃ|�C���g(gx)
				}
				//}
			  }
			}
		  }
		}

		// �����h���b�v�I�t�łЂ��������Ă���͂�
		if (_temp_enforced_dropoff_person_number != -1){ 
		  // passenger�̕��ł͂Ȃ��āA�{��(�O���[�o��)�̃C�e���[�^���g��
		  
		  list<PERSON>::iterator pos;
		  for(pos = person_list.begin(); pos!=person_list.end(); ++pos){ 
			if (pos->number == _temp_enforced_dropoff_person_number){
			  // �h���b�v�I�t�|�C���g���㏑��
			  pos->dropoff_grid.gx = _temp_enforced_dropoff_gx; 
			  pos->dropoff_grid.gy = _temp_enforced_dropoff_gy;
			}
		  }
		}

		/////////////////////////
		// pickup�|�C���g���v�Z����
		// pickup�|�C���g�́Adropoff���l��������΁A�X�V����Ȃ�
		/////////////////////////
		
		list<PERSON>::iterator pos;
		// �o�X��҂��Ă����q
		for(pos = person_list.begin(); pos!=person_list.end(); ++pos){ 
		  //		  if (pos->person_status == 2){ // 2:�o�X�ҋ@��
		  if (pos->person_status == -2){ // -2:����
			// _dep_xx �͈Ӗ����ς��Ă��邪�A�ύX����Ɩʓ|�ɂȂ�̂ł��̂܂�
			for (int i = 0; i < 1; i++){
			  _dep_gx = pos->dep_grid.gx + rnd[i].x; 
			  _dep_gy = pos->dep_grid.gy + rnd[i].y; 
			  
			  // ���E���`�F�b�N���� ( (0 <= _dep_gx <= 8) and (0 <= _dep_gy <= 4))
			  //if ((0 <= _dep_gx) && (_dep_gx <= 8) && (0 <= _dep_gy) && (_dep_gy <= 4)){
			  if ((0 <= _dep_gx) && (_dep_gx <= 9) && (0 <= _dep_gy) && (_dep_gy <= 9)){
				
				// �������O���������Ȃ� �ɉ����āA
				// ��ԑO�Ȃ̂ŏ�q�̕����͋C�ɂ���(���~�߂�)

				//if ((_dep_gx >=next_grid.gx) && 
				//((direction == pos->direction) || (pos->direction == 3))) {
				  
				// �o�X��(next_grid.gx,next_grid.gy)��(_dep_gx, _dep_gy)�̈ړ����Ԃ�
				// �ȉ��̒ʂ�
				double _bus_drive_time = 
				  d_bus_only[next_grid.gx * 10 + next_grid.gy][_dep_gx * 10 + _dep_gy];
				
				// ��q��(pos->dep_grid.gx, pos->dep_grid.gy)��(_dep_gx, _dep_gy)�̈ړ����Ԃ�
				// �ȉ��̒ʂ�
				double _person_walk_time = 
				  d_walk_only[pos->dep_grid.gx * 10 + pos->dep_grid.gy][_dep_gx * 10 + _dep_gy];
				
				// ���R�̂��ƂȂ���A��q�̕��������������Ă��Ȃ���΁A�o�X�ɏ��Ȃ�
				// ����ɊY�����Ȃ����W�́A�����Ώۂ���r�����邱�ƂɂȂ�
				if (_person_walk_time < _bus_drive_time){
				  
				  if (shortest > d_bus_only[next_grid.gx * 10 + next_grid.gy][_dep_gx * 10 + _dep_gy]){
					shortest = d_bus_only[next_grid.gx * 10 + next_grid.gy][_dep_gx * 10 + _dep_gy];
					n_gx = _dep_gx;
					n_gy = _dep_gy;
					bus_flag = 1;
					
					// �m�肵����o�X����A�w��̏�q�ɁA��Ԓn�_��������
					// �w�肳�ꂽ��q�́A���̒n�_���Apickup�Ƃ��Ď����œo�^����
					_temp_enforced_pickup_person_number = pos->number; // (��)��q�ԍ��̑ޔ�
					_temp_enforced_pickup_gx = n_gx; // (��)������ԃ|�C���g(gx)
					_temp_enforced_pickup_gy = n_gy; // (��)������ԃ|�C���g(gx)
					
				  }
				}
				// }			
			  }
			}
		  }
		}
		
		// �ȉ��͔�������Ȃ��\��������(shortest�̐��ʂ������ꍇ)
		if (_temp_enforced_pickup_person_number != -1){
		  for(pos = person_list.begin(); pos!=person_list.end(); ++pos){ 
			if (pos->number == _temp_enforced_pickup_person_number){
			  // �h���b�v�I�t�|�C���g���㏑��
			  pos->pickup_grid.gx = _temp_enforced_pickup_gx; 
			  pos->pickup_grid.gy = _temp_enforced_pickup_gy;
			}
		  }
		}
	  }// 	  if (direction == -1){ 	  // �i�s�����́A���w��
	  
	  /////////////////////////////////////////////////////////////////////////////////
	  

	  // �V���[�g�̐���
	  //// n_gx, n_gy ��shortest���ŏ��ƂȂ�^�[�Q�b�g�̍��W�͋L�^����Ă���͂�
	  // �Ƃ����̂͒����B
	  // �X�V��ɃG���g���[�͏�����̂ŁA����ς�_��
	  // �Č������܂��B�ʓ|������
	  
	  // �܂��ŏ��ɁAdropff���璲�ׂ�
	  // �ł�dropoff�́A��q�̒�����I�΂Ȃ��Ƃ����Ȃ��̂ŁA������ƉI���ɂȂ�B
	  
	  shortest = 999.9; // ���Z�b�g����
	  bus_flag = -1;
	  
	  //list<PERSON>::iterator pos;
	  for(pos = person_list.begin(); pos!=person_list.end(); ++pos){ // ��q�S��
		
		list<PERSON>::iterator bp;
		for(bp = passenger.begin(); bp!=passenger.end(); ++bp){	// �o�X�ɏ���Ă����q
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
	  } // 	  for(pos = person_list.begin(); pos!=person_list.end(); ++pos){ // ��q�S��

	  int g1 = next_grid.gx * 10 + next_grid.gy;
	  int g2 = n_gx * 10 + n_gy;

#if 0	// �ړI�n�ƌ��ݒn����v���Ă��܂����Ƃ�����  
	  if (g1 == g2){
		printf("i = %d\n",global_clock);
		exit(1);
	  }
#endif
		
	  //  if (bus_flag == 1){
	  if ((bus_flag == 1) && (g1 != g2)){
		p_bus_only_path.clear(); // ���[�g���폜����

		//int g1 = next_grid.gx * 10 + next_grid.gy;
		//int g2 = n_gx * 10 + n_gy;

		printf("g1:%d, g2:%d\n", g1, g2);
		
		copy(path_bus_only[g1][g2].begin(), path_bus_only[g1][g2].end(), 
			 back_inserter(p_bus_only_path));
		
		p_bus_only_path.pop_front(); // �擪�̗v�f���폜	
		
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

	  if (!p_bus_only_path.empty()){ // ��łȂ����
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
		p_bus_only_path.pop_front(); // (���̏����Ɉׂ�)�擪�̗v�f���폜	
		
	  }
	  else{ // ��ł����
		// printf("Bus Path is exhausted\n");
		// exit(1); // �����I�����Ă��܂��܂��傤�B
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
	printf("���������m�ۂł��܂���\n");
	exit(EXIT_FAILURE);
  }
  memset(new_p_state_seq, 0, sizeof(STATE_SEQ)); // 
  memcpy(new_p_state_seq, &present_state_seq, sizeof(STATE_SEQ)); // ���݂́u��ԁv�����
 
  // �K����state������ē\����Ă���
  //new_p_state_seq->state = make_state();
  new_p_state_seq->state = state;

  // �b��I�ȃe�X�g(�����܂�)
  

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
	STATE_SEQ* p_seq = p_state_seq->next_seq; // �|�C���^�����
	free(p_state_seq);
	p_state_seq = p_seq;
  }
  // �|�C���^�̃��Z�b�g
  p_first_state_seq->prev_seq = NULL;
  p_last_state_seq->next_seq = NULL;
  p_first_state_seq->next_seq = p_last_state_seq;
  p_last_state_seq->prev_seq = p_first_state_seq;
}


void BUS::q_reward(int final_reward)
{
  printf("passed q_reward(%d)\n", final_reward);
  
  p_last_state_seq->state->q = final_reward; // �S�[���ɋ��z���Ԃ炳���Ă���

  int number = 0;
  int f_r = final_reward;
  
  STATE_SEQ* p_state_seq = p_last_state_seq->prev_seq;  
  while (p_state_seq != p_first_state_seq){  
	/// �������e(�J�n)
	/// �t�����ɕ�V��^���Ă���

#if 1	
	p_state_seq->state->q += 
	  0.1 * (0.9 * p_state_seq->next_seq->state->q - p_state_seq->state->q);



	//p_state_seq->state->q += 
	//  0.9 * p_state_seq->state->q + 0.1 * p_state_seq->next_seq->state->q ;


#else
	p_state_seq->state->q += 10; // ��������10�_��^����A�Ƃ�����H

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
	printf("���������m�ۂł��܂���\n");
	exit(EXIT_FAILURE);
  }
  memset(p_top_state_seq, 0, sizeof(STATE_SEQ)); // �[���N���A

  // �b��I�ȃe�X�g(��������)

  // �K����state������ē\����Ă���
  p_top_state_seq->state = make_state(0,0,0);

  // �b��I�ȃe�X�g(�����܂�)

  STATE_SEQ *p_tail_state_seq = (STATE_SEQ *)malloc(sizeof(STATE_SEQ));
  if(p_tail_state_seq == NULL) {
	printf("���������m�ۂł��܂���\n");
	exit(EXIT_FAILURE);
  }
  memset(p_tail_state_seq, 0, sizeof(STATE_SEQ)); // �[���N���A


  // �b��I�ȃe�X�g(��������)

  // �K����state������ē\����Ă���
  p_tail_state_seq->state = make_state(0,0,0);

  // �b��I�ȃe�X�g(�����܂�)

  p_first_state_seq = p_top_state_seq;
  p_last_state_seq = p_tail_state_seq;

  p_first_state_seq->prev_seq = NULL;
  p_last_state_seq->next_seq = NULL;
  p_first_state_seq->next_seq = p_last_state_seq;
  p_last_state_seq->prev_seq = p_first_state_seq;

  return;
}
