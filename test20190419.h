/*
  test20190419.h
*/

#ifndef _CA_SMALL_H_
#define _CA_SMALL_H_

// ���͍]�[�̉Ƃ̍��W
const double center_x = 139.4755406;
const double center_y =  35.598697;

const double diff_x = 0.00276; // X������250���[�g��
const double diff_y = 0.00225; // Y������250���[�g��

const double pi = 3.141592654;

struct RND // round�̈Ӗ�
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
  STATE *state;  // state_list�̂ǂꂩ����Q�Ƃ���

  STATE_SEQ *prev_seq;
  STATE_SEQ *next_seq;

};


class LOCATION // �����W
{
public:
  double px;
  double py;
};

class GRID // ������W
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
    
  int direction; // 1:�� 2:�� 3:�Ȃ�

  int pickup_flag; // �f�t�H���g��0   
  int person_flag; 

  int stop_timer;
  int person_status; // 0:�o���O 1:���s�� 2:�o�X�ҋ@�� 3:��Ԓ� -1:��������(��~��)
  
  int number;
  char name[10]; // ���� �� "person5" "person15"
  
  GRID dep_grid; // �o���n(������W)
  GRID arr_grid; // �����n(������W) 

  GRID pickup_grid; // �o�X�̃s�b�N�A�b�v�n(������W)
  GRID dropoff_grid; // �o�X�̃s�b�N�A�b�v�n(������W)
  
  LOCATION departure; // �o���n
  LOCATION arrival; // �����n
  
  GRID prev_grid; // �O�̌����_(������W)
  GRID next_grid; // ���̌����_(������W) 
  
  LOCATION prev_person_stop; // �O�̌����_(�ܓx�A�y�x)
  LOCATION present;  // ���݈ʒu(�ܓx�A�y�x)
  LOCATION next_person_stop; //���̌����_(�ܓx�A�y�x)
  
  BUS *bus; // ���ݏ�Ԃ��Ă���o�X

  int on_the_bus; // ���ݏ�Ԃ��Ă���o�X(�̔ԍ�)
  
  list<int> p_walk_and_bus_path; // ���s�ƃo�X���g�����ꍇ�̌o�H
  list<int> p_walk_only_path; // �������ꍇ�̌o�H

  int gen_time; // ��������
  int dep_time; // �o������
  int home_time;  // ����ҋ@����
  int arr_time; // ��������
  int travel_time; // �ړ�����

  int life_time; // ����������ł܂ł̎���

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

  int number; // �o�X�̃��j�[�N�Ȕԍ�

  int direction; // 1:�� 2:��
  
  GRID prev_grid; // �O�̃o�X�◯��(������W)
  GRID next_grid; // ���̃o�X�◯��(������W) 

  LOCATION prev_bus_stop; // �O�̃o�X�◯��(�ܓx�A�y�x)
  LOCATION present;  // ���݈ʒu(�ܓx�A�y�x)
  LOCATION next_bus_stop; //���̃o�X�◯��(�ܓx�A�y�x)
  
  int stop_timer;
  int bus_status;

  list<int> p_bus_only_path; // �o�X���g�����ꍇ�̌o�H

  list<PERSON> passenger;  // ���̃o�X�ɏ���Ă����q�̃��X�g

  double shortest;
  int bus_flag; 
  int n_gx, n_gy, _arr_gx, _arr_gy, _dep_gx, _dep_gy;

  int _temp_enforced_pickup_person_number;
  int _temp_enforced_pickup_gx;
  int _temp_enforced_pickup_gy;

  int _temp_enforced_dropoff_person_number;
  int _temp_enforced_dropoff_gx;
  int _temp_enforced_dropoff_gy;

  //// 12��Ԃɂ܂ň��k (�߂��ɂ����Ԃ���ԏd���Ɣ��f����)
  // "3"�̈Ӗ��� Waiting Person | Riden Person | Bus. 
  // "4"�̈Ӗ� -> 45�`135�x | 135�`225�x | 225 �` 315�x | 315 �` 45�x
  // �l�̈Ӗ�3:360���[�g���ȓ��A2: 720���[�g���ȓ��A1:1080���[���ȓ� 0: ���Ȃ�
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

//// d[100][100]�̑ޔ�ϐ�



#endif // _CA_SMALL_H_
