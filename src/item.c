/*
 *   file: item.c
 *   date: 11/12/2014
 *   auth: trickyloki3
 * github: https://github.com/trickyloki3
 *  email: tricky.loki3@gmail.com
 */
#include "api.h"
#include "script.h"
int load_by_mode(int mode, struct ic_db_t * db, ic_item_t * item);

int main(int argc, char * argv[]) {
	global_mode = EATHENA;
	int sqlite_status = 0;
	int script_status = 0;
	char err[BUF_SIZE];

	ic_item_t item;
	token_r token_list;
	int block_cnt = 0;
	block_r * block_list;
	global_db = init_ic_db("athena.db");

	sqlite_status = load_by_mode(global_mode, global_db, &item);
	while(sqlite_status == SQLITE_ROW) {
		block_cnt = 0;
		block_init(&block_list, BLOCK_SIZE);
		if(strlen(item.script) > 0) {
			/*printf("%d:%s;%s\n", item.id, item.name, item.script); fflush(NULL);*/

			/* perform lexical analysis */
			script_status = script_lexical(&token_list, item.script);
			if(script_status == SCRIPT_FAILED) {
				exit_abt(build_buffer(err, "lokic: failed to perform lexical analysis on item %d\n", item.id));
			} else {
				/* perform structual analysis */
				script_status = script_analysis(&token_list, block_list, &block_cnt, item.id, 0x01, 0);
				if(script_status == SCRIPT_FAILED) {
					exit_abt(build_buffer(err, "lokic: failed to perform structual analysis on item %d\n", item.id));
				} else if(block_cnt > 0) {
					/* perform set block inheritance */
					script_dependencies(block_list, block_cnt);
					/* translate the script */
					script_translate(block_list, block_cnt);
				}
			}
		}

		block_deinit(block_list, BLOCK_SIZE);
		sqlite_status = load_by_mode(global_mode, global_db, &item);
	}
	deit_ic_db(global_db);
	return 0;
}

int load_by_mode(int mode, struct ic_db_t * db, ic_item_t * item) {
	int status = 0;
	switch(mode) {
		case EATHENA:
			status = sqlite3_step(db->ea_item_iterate);
			if(status == SQLITE_ROW) {
				item->id = sqlite3_column_int(db->ea_item_iterate, 0);
				if(item->name != NULL) free(item->name);
				item->name = convert_string((const char *) sqlite3_column_text(db->ea_item_iterate, 2));
				if(item->script != NULL) free(item->script);
				item->script = convert_string((const char *) sqlite3_column_text(db->ea_item_iterate, 19));
			}
			break;
		case RATHENA:
			status = sqlite3_step(db->ra_item_iterate);
			if(status == SQLITE_ROW) {
				item->id = sqlite3_column_int(db->ra_item_iterate, 0);
				if(item->name != NULL) free(item->name);
				item->name = convert_string((const char *) sqlite3_column_text(db->ra_item_iterate, 2));
				if(item->script != NULL) free(item->script);
				item->script = convert_string((const char *) sqlite3_column_text(db->ra_item_iterate, 20));
			}
			break;
		case HECULES:
			status = sqlite3_step(db->he_item_iterate);
			if(status == SQLITE_ROW) {
				item->id = sqlite3_column_int(db->he_item_iterate, 0);
				if(item->name != NULL) free(item->name);
				item->name = convert_string((const char *) sqlite3_column_text(db->he_item_iterate, 2));
				if(item->script != NULL) free(item->script);
				item->script = convert_string((const char *) sqlite3_column_text(db->he_item_iterate, 39));
			}
			break;
	}
	return status;
}