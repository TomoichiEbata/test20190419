/*
  g++ -g v2_ebata99_csv.cpp -o v2_ebata99_csv.exe
*/

#include <stdio.h>

class point{
  double x;
  double y;
};

const double diff_x = 0.00276;
const double diff_y = 0.00225;

const double center_x = 139.4755406;
const double center_y =  35.598697;


#if 0
<node id="11" visible="true" version="4" changeset="35026937" timestamp="2019-02-19T17:09:00Z" user="ebata" uid="9999" lat="35.594197" lon="139.464501"/>

<node id="95" visible="true" version="4" changeset="35026937" timestamp="2019-02-19T17:09:00Z" user="ebata" uid="9999" lat="35.603197" lon="139.486581"/>

 <way id="25534234" visible="true" version="21" changeset="54896583" timestamp="2017-12-24T21:40:42Z" user="machiro" uid="4480756">
  <nd ref="278288867"/>
  <nd ref="278288868"/>
  <nd ref="3813452336"/>
  <nd ref="280394410"/>
</way>

#endif

int main()
{
#if 0

  printf("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
  printf("<osm version=\"0.6\" generator=\"CGImap 0.6.1 (1919 thorn-01.openstreetmap.org)\" copyright=\"OpenStreetMap and contributors\" attribution=\"http://www.openstreetmap.org/copyright\" license=\"http://opendatacommons.org/licenses/odbl/1-0/\">\n");
	
  //ç∂â∫Ç∆âEè„ÇíËã`Ç∑ÇÍÇŒó«Ç¢
  //<node id="-11" visible="true" version="1" timestamp="2019-02-19T17:09:00Z" user="ebata" uid="9999" lat="35.594197" lon="139.464501"/>
  //<node id="-110" visible="true" version="1" timestamp="2019-02-19T17:09:00Z" user="ebata" uid="9999" lat="35.614447" lon="139.489341"/>
  //printf("<bounds minlat=\"%f\" minlon=\"%f\" maxlat=\"%f\" maxlon=\"%f\"/>\n", 
  // 35.594197 - diff_y, 139.464501 - diff_x, 35.603197 + diff_y, 139.486581 + diff_x );

  printf("<bounds minlat=\"%f\" minlon=\"%f\" maxlat=\"%f\" maxlon=\"%f\"/>\n", 
		 35.594197 - diff_y, 139.464501 - diff_x, 35.614447 + diff_y, 139.489341 + diff_x );

#endif 
  // for (int y = -2; y < 3; y++){ // 0Å`4 Å® Ç±ÇÍÇ0Å`9Ç…Ç∑ÇÈÇ∆Ç¢Ç§Ç±Ç∆ÇÕÅA5Çë´ÇπÇŒÇ¢Ç¢(ÇÕÇ∏)
  for (int y = -2; y < 3 + 5; y++){ // 0Å`4 + 5
	//for (int x = -4; x < 5; x++){ // 0Å`8  Å® Ç±ÇÍÇ0Å`9Ç…Ç∑ÇÈÇ∆Ç¢Ç§Ç±Ç∆ÇÕÅA1Çë´ÇπÇŒÇ¢Ç¢
	for (int x = -4; x < 5 + 1; x++){ // 0Å`8 + 1
	  double px = center_x + diff_x * (double)x;
	  double py = center_y + diff_y * (double)y;

	  //int id = ((x + 5) * 10 + (y + 3)) * (-1);
	  int id = ((x + 4) * 10 + (y + 2)) ;

	  printf("%02d,%f,%f\n", id,px,py);
#if 0
 
	  //printf("<node id=\"%d\" visible=\"true\" version=\"1\" changeset=\"35026937\" timestamp=\"2019-02-19T17:09:00Z\" user=\"ebata\" uid=\"9999\" lat=\"%f\" lon=\"%f\"/>\n",id, py, px); 
	  printf("<node id=\"%d\" visible=\"true\" version=\"1\" timestamp=\"2019-02-19T17:09:00Z\" user=\"ebata\" uid=\"9999\" lat=\"%f\" lon=\"%f\"/>\n",id, py, px); 

#endif
	}
  }

#if 0
  int way_id = -200;

  // for (int y = -2; y < 3; y++){ // 0Å`4 Å® Ç±ÇÍÇ0Å`9Ç…Ç∑ÇÈÇ∆Ç¢Ç§Ç±Ç∆ÇÕÅA5Çë´ÇπÇŒÇ¢Ç¢(ÇÕÇ∏)
  for (int y = -2; y < 3 + 5; y++){ // 0Å`4 + 5
	//for (int x = -4; x < 5; x++){ // 0Å`8  Å® Ç±ÇÍÇ0Å`9Ç…Ç∑ÇÈÇ∆Ç¢Ç§Ç±Ç∆ÇÕÅA1Çë´ÇπÇŒÇ¢Ç¢
	for (int x = -4; x < 5 + 1; x++){ // 0Å`8 + 1
	  int new_y = y + 3;
	  int new_x = x + 5;

	  int id = (new_x * 10 + new_y) * (-1); 


	  //if (new_x < 9){
	  if (new_x < 10){
		way_id -= 1;
		// printf("<way id=\"%d\" visible=\"true\" version=\"1\" changeset=\"54896583\" timestamp=\"2019-02-19T17:09:00Z\" user=\"ebata\" uid=\"9999\">\n", way_id);
		printf("<way id=\"%d\" visible=\"true\" version=\"1\" timestamp=\"2019-02-19T17:09:00Z\" user=\"ebata\" uid=\"9999\">\n", way_id);
		printf("<nd ref=\"%d\"/>\n", ((new_x + 0) * 10 + (new_y + 0)) * (-1)); 
		printf("<nd ref=\"%d\"/>\n", ((new_x + 1) * 10 + (new_y + 0)) * (-1)); 
		printf("<tag k=\"highway\" v=\"steps\"/>\n");
		printf("</way>\n");
	  }

	  //if ( new_y < 5 ){
	  if ( new_y < 10 ){
		way_id -= 1;
		//printf("<way id=\"%d\" visible=\"true\" version=\"1\" changeset=\"54896583\" timestamp=\"2019-02-19T17:09:00Z\" user=\"ebata\" uid=\"9999\">\n", way_id);
		printf("<way id=\"%d\" visible=\"true\" version=\"1\" timestamp=\"2019-02-19T17:09:00Z\" user=\"ebata\" uid=\"9999\">\n", way_id);
		printf("<nd ref=\"%d\"/>\n", ((new_x + 0) * 10 + (new_y + 0)) * (-1)); 
		printf("<nd ref=\"%d\"/>\n", ((new_x + 0) * 10 + (new_y + 1)) * (-1)); 
		printf("<tag k=\"highway\" v=\"steps\"/>\n");
		printf("</way>\n");
	  }

	}
  }	 

  printf("</osm>\n");

#endif
  return 0;
}

