#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <mysql.h>

int main(int argc, char **argv)
{
	MYSQL mysql;
	MYSQL_RES *res;
	MYSQL_ROW row;
	char *query;
	int t;
	int r;

	mysql_init(&mysql);
	
	if(!mysql_real_connect(&mysql, "localhost", "root", "mysql", "mysql", 0, NULL, 0)) {
		printf("Error connection to database: %s\n", mysql_error(&mysql));
	}else{
		printf("Connected...\n");
	}

	//query = "insert into zhangjie(name, age, address)values(\'lijie\',25,\'keChuangRode 3th\')";
	query = "select * from zhangjie";
	t = mysql_real_query(&mysql, query, (unsigned int)strlen(query));

	if(t) {
		printf("Error, makeing query:%s\n", mysql_error(&mysql));
	}else{
		printf("[%s]made...", query);
	}

	res = mysql_store_result(&mysql);
	
	while(row = mysql_fetch_row(res)) {
		for (t = 0; t < mysql_num_fields(res); t++) {
			printf("%s ", row[t]);
		}
		printf("\n");
	}

	printf("mysql_free_result ...\n");
	//mysql_free_result(res);

	sleep(1);

	query = "insert into zhangjie(name, age, address)values('lijie',25,'keChuangRode 3th')";
	
	t = mysql_real_query(&mysql, query, (unsigned int)strlen(query));
	
	if(t) {
		printf("Error, makeing query:%s\n", mysql_error(&mysql));
	}else{
		printf("[%s]made...", query);
	}
	
	res = mysql_store_result(&mysql);
	
	while(row = mysql_fetch_row(res)) {
		for (t = 0; t < mysql_num_fields(res); t++) {
			printf("%s ", row[t]);
		}
		printf("\n");
	}
	
	mysql_free_result(res);

	mysql_close(&mysql);
	
	return 0;
	
	
}
