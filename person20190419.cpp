/*

  g++ -c person20190419.cpp

  person_status�̐���

  // person_status = -2; // ����
  // person_status = -1; // �i�v��~ 
  // person_status = 0; // ��~:0
  // person_status = 1; // 0���s:1 
  // person_status = 2; // �o�X�ҋ@��
  ((person_status == 3) && (bus->number == on_the_bus)){ // �o�X��Ԓ�

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


list<PERSON> person_list; // �V�~�����[�^��ɑ��݂���S�Ă̐l��
extern LOCATION bus_stop[10][10];
extern int global_clock; // main �̒���int i�Ɠ����l�ɂ���

extern int sd;
extern struct sockaddr_in addr;
extern WSADATA wsaData;


PERSON::PERSON(int num,
			   const char* n, 
			   int dep_grid_gx, int dep_grid_gy, int arr_grid_gx, int arr_grid_gy) 
{
  gen_time = global_clock; // main �� int i�̒l������

  // person_status = 0; // ���s:1 ��~:0
  // person_status = 2; // �ŏ�����o�X��҂�

#if 0 // Q�w�K�̎��́A������"0"�ɂ���
  person_status = -2; // ����(-2)��V��
#else 
  person_status = 2; // (�����Ȃ�)�o�X�ҋ@��
#endif

  dep_time = -1;
  home_time = -1;
  arr_time = -1; 
  travel_time = -1;
  life_time = -1;

#if 0
  if ( arr_grid_gx > dep_grid_gx) 
	direction = 1; // ������
  else if ( arr_grid_gx < dep_grid_gx)
	direction = 2; // ������
  else 
	direction = 3; // ��������
#endif //0 
  
  direction = -1; // ���w��

  on_the_bus = -1; // �ŏ��̓o�X�Ƃ̃����N�͂Ȃ�
  
  pickup_flag = 0; // �f�t�H���g��0   
  person_flag = 0; // �t���O��0�ŏグ�Ă���
  
  number = num; // ��q�ԍ�
  
  //bus = b;  // �|�C���^�Ń����N
  
  strcpy(name, n); //���O�̃R�s�[
  
  stop_timer = 2; // �����_�̃X�e�b�v��
  
  // �悸�o���n�Ɠ����n�̃O���b�h�ƍ��W������
  dep_grid.gx = dep_grid_gx;
  dep_grid.gy = dep_grid_gy;
  arr_grid.gx = arr_grid_gx;
  arr_grid.gy = arr_grid_gy;
  
  departure.px = bus_stop[dep_grid_gx][dep_grid_gy].px;
  departure.py = bus_stop[dep_grid_gx][dep_grid_gy].py;
  arrival.px = bus_stop[arr_grid_gx][arr_grid_gy].px;
  arrival.py = bus_stop[arr_grid_gx][arr_grid_gy].py;
  
  // �����_�̏����l���Œ�
  prev_grid.gx = next_grid.gx = dep_grid_gx;
  prev_grid.gy = next_grid.gy = dep_grid_gy;
  
  present.px = prev_person_stop.px = next_person_stop.px 
	= bus_stop[dep_grid_gx][dep_grid_gy].px;
  present.py = prev_person_stop.py = next_person_stop.py 
	= bus_stop[dep_grid_gx][dep_grid_gy].py;
 
  // �s�b�N�A�b�v�|�C���g�ƃh���b�v�I�t�|�C���g�̃f�t�H���g�ݒ�(-1,-1)

#if 0 // Q�w�K�̎��́A������"0"�ɂ���
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
  
  //  printf("%s",ch); // �m�F�p
  
  if (sendto(sd, ch, strlen(ch), 0, (struct sockaddr *)&addr,  sizeof(addr)) < 0) {        
	perror("sendto");
	printf("error:%d\n",WSAGetLastError());
	return -1;
  }
  return 0;
}


int PERSON::person_change_direction(int next_grid_gx, int next_grid_gy)
{ // ���̌o�H������
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

// status 1, -1, 0,-2 ��4��Ԃ�����
int PERSON::walk_only_check()
{
  //bus = b;  // �|�C���^�Ń����N	
  /////////////////////////////////////////////////////////////////////////////  
  if (person_status == 1){ // 1:���s��
	/////////////////////////////////////////////////////////////////////////////
	
	// �܂��A1�X�e�b�v��(3.6�b)��i�߂�
	// const double diff_x = 0.00276; // X������250���[�g��
	// const double diff_y = 0.00225; // Y������250���[�g��
	// 1�X�e�b�v��3.6m �i�� �� ����3.6km ������A
	// �� (3.6 / 250.0)����Z����
	
	double rad = atan2((next_person_stop.py - present.py), 
					   (next_person_stop.px - present.px));
	// present���X�V����
	present.py += diff_y * (3.6 / 250.0) * sin(rad);
	present.px += diff_x * (3.6 / 250.0) * cos(rad);
	
	double rad_0 = atan2((next_person_stop.py - prev_person_stop.py),
						 (next_person_stop.px - prev_person_stop.px));
	
	double rad_1 = atan2((next_person_stop.py - present.py), 
						 (next_person_stop.px - present.px));
	
	
	// ���X�e�[�^�X�`�F���W�̐؂��|��
	if (fabs(rad_0 -rad_1) >= pi * 0.5 - 0.0001){ // �����_���z����
	  
	  // �����_�Ɉʒu���Œ肷��
	  present.py = next_person_stop.py;
	  present.px = next_person_stop.px;
	  
	  // �ꎞ��~(0)�Ƀ`�F���W
	  person_status = 0;
	  printf("\nnum=%d:person_status = 1->0\n", number);
	} // f (fabs(rad_0 -rad_1) >= pi * 0.5 - 0.0001){ // �����_���z����
  } // if (person_status == 1){ // 1:���s��

  /////////////////////////////////////////////////////////////////////////////  
  else if (person_status == -1){ // �i�v��~ 
  /////////////////////////////////////////////////////////////////////////////
	if (arr_time == -1){ // �����������L�^
	  arr_time = global_clock;
	  travel_time = arr_time - dep_time;
	}

	person_status = -1; // �i�v��~����͔������Ȃ�
	// ���X�g����O�� // �T�u���[�`���̊O�ō폜����

	//printf("\n -1 -> -1\n"); 
  } // status == -1 

  /////////////////////////////////////////////////////////////////////////////  
  else if (person_status == -2){ // ����
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
  else if (person_status == 0){ // �ꎞ��~
  /////////////////////////////////////////////////////////////////////////////
	stop_timer -= 1; // �X�e�b�v���Ԃ�1���Z
	
	if ( stop_timer < 0){ // �X�e�b�v���Ԃ��g���ʂ�����X�e�[�^�X�������o��
	  
	  stop_timer = 2; // �X�e�b�v����߂�
	  // ���݈ʒu��next_grid.gx,next_grid.gy
	  
	  // �s�b�N�A�b�v���o�^����Ă��邪�h���b�v�I�t�͓o�^����Ă��Ȃ�
	  if ((pickup_grid.gx != -1) && (dropoff_grid.gx == -1)){
		if ((next_grid.gx == pickup_grid.gx) && (next_grid.gy == pickup_grid.gy)){
		  person_status = 2;
		  printf("\nnum=%d:person_status = 0->2\n", number);
		}
	  }

	  // �s�b�N�A�b�v�A�h���b�v�I�t���o�^����Ă��čŏI�����l�Ȃ�i�v��~
	  if ((pickup_grid.gx != -1) && (dropoff_grid.gx != -1)) {
		if ((next_grid.gx == arr_grid.gx) && (next_grid.gy == arr_grid.gy)){// ����
		  person_status = -1;
		  printf("\nnum=%d:person_status = 0->-1\n", number);
		  printf("next_grid.gx=%d, arr_grid.gx=%d next_grid.gy=%d,arr_grid.gy=%d\n",
				 next_grid.gx, arr_grid.gx,next_grid.gy,arr_grid.gy);
		}
	  }

	  // �s�b�N�A�b�v�A�h���b�v�I�t���o�^����Ă��ŏI�����łȂ��Ȃ����
	  if ((pickup_grid.gx != -1) && (dropoff_grid.gx != -1)) {
		if ((next_grid.gx != arr_grid.gx) || (next_grid.gy != arr_grid.gy)){//������
		  person_status = 1;
		  person_change_direction(arr_grid.gx,arr_grid.gy);
		  printf("\nnum=%d:person_status = 0->1\n", number);
		  printf("next_grid.gx=%d, arr_grid.gx=%d next_grid.gy=%d,arr_grid.gy=%d\n",
				 next_grid.gx, arr_grid.gx,next_grid.gy,arr_grid.gy);
		  
		}
	  }
	}
  } //   else if (person_status == 0){ // �ꎞ��~

  return 0;
}

// status 2, 3 ��2��Ԃ�����
int PERSON::walk_and_bus_check(list<BUS>::iterator bus)
{
  /////////////////////////////////////////////////////////////////////////////
  if (person_status == 2){ // �o�X�ҋ@��
  /////////////////////////////////////////////////////////////////////////////
	// ���̃P�[�X�ł́A��q�Ɏ������s�̈ӎv�͂Ȃ��Ƃ���
	// �o�X�̕������ƁA��q�̕���������v���Ă���΁A���΂悢
	// ��q�Ɏ��R�ӎv�͂Ȃ�
	
	// �o�X�̕����ƈړ���������v���Ă��āA
	if((bus->bus_status == 0) && (bus->direction ==  direction)){
	  // �o�X�̈ʒu�Ƒҋ@���̈ʒu���ꏏ�ł����
	  // �o�X�̌��݈ʒu(���W)�́Abus->next_grid.gx, bus->next_grid.gy
	  // ��q�̌��݈ʒu(���W)�́Anext_grid.gx, next_grid.gy
	  
	  printf("bus status = 2 Bus:%d,%d, Person:%d,%d\n",
			 bus->next_grid.gx,
			 bus->next_grid.gy,
			 next_grid.gx,
			 next_grid.gy);
	  
	  if ((bus->next_grid.gx == next_grid.gx) && (bus->next_grid.gy == next_grid.gy)){
		on_the_bus = bus->number; // �o�X�Ƃ̃����N�𒣂�


		//���̒i�K�ł�dropoff_grid�����m��̉\��������(�Ƃ������m���ɂ������낤)
		// �Ȃ̂� change_direction(dropoff_grid.gx, dropoff_grid.gy) �͕ʂ̂Ƃ���ł��K�v������
		//change_direction(dropoff_grid.gx, dropoff_grid.gy);		

		person_status = 3;
		printf("\nnum=%d:person_status = 2->3\n", number);
		printf("bus_num=%d\n", bus->number);		

		// �Ō�ɓ����
		bus->passenger.push_back(*this); // �o�X�̏�q

	  }
	}
  } // status == 2
  	//else if (status == 3){ // �o�X��Ԓ�
  /////////////////////////////////////////////////////////////////////////////
  else if ((person_status == 3) && (bus->number == on_the_bus)){ // �o�X��Ԓ�
  /////////////////////////////////////////////////////////////////////////////

	if (dep_time == -1){
	  dep_time = global_clock;
	  home_time = dep_time - gen_time;
	}

	//static int flag = 0;  //�����t���O�͓|���Ă���
	
	if (bus->bus_status == 1){ // �o�X�ړ���
	  // present���o�X�̈ړ��ɘA��������
	  present.py = bus->present.py;
	  present.px = bus->present.px;
	  
	  // �ړ����ɁAdropoff_grid���m�肷��͂��Ȃ̂ŁA���̃^�C�~���O��(1�񂾂�)�o�^����
	  if ((dropoff_grid.gx != -1) && (pickup_flag == 0)){
		pickup_flag = 1;
		person_change_direction(dropoff_grid.gx, dropoff_grid.gy);		
		printf("dropoff_grid is entried %d:%d\n",dropoff_grid.gx, dropoff_grid.gy);
	  }

	  person_flag = 1; // �t���O�𗧂đ�����
	}// 	if (bus->bus_status == 1){ // �o�X�ړ���

	else if (bus->bus_status == 0){ // �o�X��Ԓ�(���Ԃ���o�X�₩�ۂ�)
	  // 1�񂾂��ʉ߂����邽�߂Ƀt���O���g��
	  if (person_flag == 1){
		//printf("bus and person stop \n");
		person_flag = 0; // �t���O��|��

		// �o�X�̈ʒu�Ə�q�̍~�Ԉʒu����v���Ă���΁A���Ԃ���
		// �o�X�̌��݈ʒu(���W)�́Abus->next_grid.gx, bus->next_grid.gy
		// ��q�̌��݈ʒu��arr_grid.gx, arr_grid.gy

#if 0
		// �����A���̒i�K�ł��Adropoff_grid�����肵�Ă��Ȃ�������A�o�O
		if (dropoff_grid.gx == -1){
		  printf("Bug; dropoff_grid is not fixed!");
		  exit(-1);
		}
#endif
		if (dropoff_grid.gx != -1){ // ���̒i�K�ŁAdropoff_grid�����肵�Ă��Ȃ����Ƃ�����	
		  if ((bus->next_grid.gx == dropoff_grid.gx) && (bus->next_grid.gy == dropoff_grid.gy)){
			person_status = 0;		  
			
			//dropoff_flag = 1; // dropoff_flag���グ��
			
			printf("\nnum=%d:person_status = 3->0\n", number);
			printf("bus->next_grid.gx=%d, dropoff_grid.gx=%d bus->next_grid.gy=%d, dropoff_grid.gy=%d\n",bus->next_grid.gx,dropoff_grid.gx,bus->next_grid.gy, dropoff_grid.gy);
			printf("arr_grid.gx=%d arr_grid.gy=%d\n", arr_grid.gx, arr_grid.gy);
			printf("bus->number=%d\n",bus->number);
			
			on_the_bus = -1; // �o�X�Ƃ̃����N��؂�
			
			// �o�X��q���X�g����̍폜
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
	}//	else if (bus->bus_status == 0){ // �o�X��Ԓ�(���Ԃ���o�X�₩�ۂ�)

	else {
	  printf("exit in 519\n");
	  exit(-1);
	}
  } //   else if ((person_status == 3) && (bus->number == on_the_bus)){ // �o�X��Ԓ�
  return 0;
}
