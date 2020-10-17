/*

  wf.cpp���x�[�X�ɍ�����\�[�X�R�[�h �� [9x9]�Ɋg��

  g++ -c route99.cpp 
  �ŃR���p�C���`�F�b�N���āA���C�����[�`���Ƀ����N����(dll�͖ʓ|�Ȃ̂ł��Ȃ�)

  ���΂炭�̓e�X�g�ׂ̈ɁAint main() ������
  g++ -g route99.cpp -o route99
  �Ƃ���

  ���������Ĕėp����ڎw���Ȃ�(�ǂ����Y��Ă��܂�)
  ���p�����[�^�́A���ڐ��l������悤�ɂ��āA�Ȃ�ł��N���X��\���̓n���ɂ��Ȃ�
  (�f�o�b�O�Ŏ��ʂ���)

  ���m�[�h�ԍ���"0-99"�܂ł����g���Ȃ��ł���(�ǂ��������Ȃ񂾂���)
  �����W�̔ԍ��́A (x,y)=(8,7) �Ȃ�A"87"�ԂƂ���A�Ƃ������ՂȌ��ߕ��ɂ���
  (�]���āAx,y�Ƃ��ɁA���W�́A0,1,2,3,4,5,6,7,8,9 �����g���Ȃ�)

  ���m�[�h�ԍ����X�p�E�X(�X�J�X�J)�ɂȂ��Ă��A�C�ɂ��Ȃ�
 
*/
 
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <list>   // list ���p�̂���

using namespace std;

//int d[100][100];  // d[i][k]�F�m�[�hi����m�[�hk�ւ̈ړ����� 
//int via[100][100];  // d[i][k]�̊Ԃɂ���(�����Ƃ�1��)���p�m�[�h

double d[100][100];  // d[i][k]�F�m�[�hi����m�[�hk�ւ̈ړ����� 
int via[100][100];  //  d[i][k]�̊Ԃɂ���(�����Ƃ�1��)���p�m�[�h

//list<int> path[100][100];   // int �^�� list ��錾  
 

#if 1
// ���p�p�X�̕\�����[�`��(�ċA�ďo���p)
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

// ���p�p�X�̕\�����[�`��(�ċA�ďo���p)
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


 
// ���p�p�X�̕\�����[�`��
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
  // �ϐ��̏����� (�S���̗v�f�𖄂߂Ă���̂ŁA�[�����Z�b�g����K�v�͂Ȃ�)
  for(int i = 0; i < 100; i++){
	for(int j = 0; j < 100; j++){
	  d[i][j] = 999.9; // �ړ����Ԃ̏�����(��펯�Ȃ��炢�ł������l����͂��Ă���(INT_MAX�͑����Z�̎��Ɍ��オ�肪�N����̂Ŏg��Ȃ�)
	  via[i][j] = i; // �m�[�hi����m�[�hk�ւ̌o�R�l�̏����� 
	}
  }
  
  for(int i = 0; i < 100; i++){
	d[i][i] = 0; //// �����m�[�h�ւ̈ړ����Ԃ�0�ɂȂ�̂ŁA�㏑��
  }
  
  //�m�[�h�ԍ��̒ʔԂ��ȉ��̂悤�ɂ���
  // [0][2] �� "02", [4][9] �� "49", [9][[9] �� "99"
  // ���W��1�P�^���ɗ��߂�
  
  // �ȉ���0.069�Ƃ́A250���[�g��������3.6km�ŕ��������́A����(0.069����,4.2���A250�b)�̂��Ƃł���B
  
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
  // �o�X�̈ړ����� 
  // �ȉ���0.025�Ƃ́A250���[�g��������10km�ő��s�������́A����(0.025���ԁA1.5���A90�b)�̂��Ƃł���B
  // �����p�㏑��(�������L���ł�)
  
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
  // �ϐ��̏����� (�S���̗v�f�𖄂߂Ă���̂ŁA�[�����Z�b�g����K�v�͂Ȃ�)
  for(int i = 0; i < 100; i++){
	for(int j = 0; j < 100; j++){
	  d[i][j] = 999.9; // �ړ����Ԃ̏�����(��펯�Ȃ��炢�ł������l����͂��Ă���(INT_MAX�͑����Z�̎��Ɍ��オ�肪�N����̂Ŏg��Ȃ�)
	  via[i][j] = i; // �m�[�hi����m�[�hk�ւ̌o�R�l�̏����� 
	}
  }
  
  for(int i = 0; i < 100; i++){
	d[i][i] = 0; //// �����m�[�h�ւ̈ړ����Ԃ�0�ɂȂ�̂ŁA�㏑��
  }

  //�m�[�h�ԍ��̒ʔԂ��ȉ��̂悤�ɂ���
  // [0][2] �� "02", [4][9] �� "49", [9][[9] �� "99"
  // ���W��1�P�^���ɗ��߂�

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
  // �o�H���v�Z
  for (int k =0; k < 99; k++){  
	for (int i =0; i < 99; i++){
	  for(int j = 0; j < 99; j++){
		if(d[i][j] > d[i][k] + d[k][j]){
		  d[i][j] = d[i][k] + d[k][j];
		  via[i][j] = k; //�X�V����
		}
	  }
	}
  }
  return 0;
#endif 

  // �o�H���v�Z
  for (int k =0; k < 100; k++){  
	for (int i =0; i < 100; i++){
	  for(int j = 0; j < 100; j++){
		if(d[i][j] > d[i][k] + d[k][j]){
		  d[i][j] = d[i][k] + d[k][j];
		  via[i][j] = k; //�X�V����
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

  // path_walk��2�����z������̂܂܃T�u���[�`���Ɏ����Ă����͓̂���̂ŁA
  // ���C���ŉ񂷂��Ƃɂ���(���X�J�b�R������)

  list<int> path_walk[100][100];   // int �^�� list ��錾  

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
  make_route_change(); // �H�����e�̋����㏑��
  make_route_calc();

  list<int> path_bus[100][100];   // int �^�� list ��錾  

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

  // path_walk , path_bus�̓}�X�^�Ƃ��Ďg���āA���ۂ̃��[�g��M��Ƃ��́A
  // �ȉ��̗p�ɃR�s�[���Ďg������

  list<int> p_bus = path_bus[83][04];
  list<int> p_walk = path_walk[83][04];

  list<int> p_bus_1(p_bus.size());
  copy(p_bus.begin(), p_bus.end(), p_bus_1.begin());

  list<int> p_walk_1(p_walk.size());
  copy(p_walk.begin(), p_walk.end(), p_walk_1.begin());

  // �C�e���[�^ (�����q) �̒�`
  list<int>::iterator pos;
  for(pos = p_bus_1.begin(); pos!=p_bus_1.end(); ++pos){
      cout << *pos << "\n";
  }

  for(pos = p_walk_1.begin(); pos!=p_walk_1.end(); ++pos){
      cout << *pos << "\n";
  }


#if 0
  // https://cpprefjp.github.io/reference/algorithm/copy.html
  // back_inserter ���g���� l2 �֐ݒ�B
  // back_inserter �͗v�f���R�s�[����Ƃ��� l2.push_back() ����C�e���[�^�����֐��B
  std::list<int> l2;
  std::copy(l.begin(), l.end(), back_inserter(l2));

  // l2.erase(v.begin() + 2);       //  3�Ԗڂ̗v�f�i9�j���폜
  l2.erase(l2.begin());       // �擪�̗v�f���폜


  for(pos = l2.begin(); pos!=l2.end(); ++pos){
      cout << *pos << "\n";
  }
#endif
}

#endif
