#include "reader_chunks.h"

int reader_open_book(APP* app, char* book_path)
{
	BOOK_TYPE book_type = reader_get_book_type(book_path);


	if(book_type != BOOK_TYPE_NONE)
	{
		char* book_hash		= reader_get_book_hash(app, book_path);
		int64_t book_index	= -1;

		reader_books_db_get_index_by_hash(app, book_hash, &book_index);
		if(book_hash != NULL)
		{
			app->book_type			= book_type;
			app->book_index			= book_index;
			app->book_hash			= book_hash;
			app->book_img_table		= g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_object_unref);
			app->book_id_table		= g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);		// integer inside table

			//*****************************************************
			if(book_type == BOOK_TYPE_FB2)
				parse_fb2_file(app, book_path);
			//*****************************************************
			else if(book_type == BOOK_TYPE_FB2_ZIP)
				parse_fb2_zip_file(app, book_path);
			//*****************************************************

			if(book_index != -1)
			{
				int line				= 0;
				int line_offset			= 0;
				reader_books_db_get_int_by_index(app, book_index, "line", &line);
				reader_books_db_get_int_by_index(app, book_index, "line_offset", &line_offset);

				reader_scroll_restore(app, line, line_offset);
			}
		}
		else
			g_warning("Failed to generate book hash");

		return EXIT_SUCCESS;
	}

	return EXIT_FAILURE;		// память нужно освободить руками
}

int _strncmpr(const char *str1, const char *str2, size_t count)
{
	size_t str1_len = strlen(str1);
	if(str1_len == 0)
	{
		return -1;
	}
	size_t str2_len = strlen(str2);
	if(str2_len == 0)
	{
		return 1;
	}

	str1 += str1_len-1;
	str2 += str2_len-1;

	size_t pos	= MIN(str1_len, str2_len);

	pos			= MIN(pos, count);

	while(pos != 0)
	{
		if(*str1 != *str2)
			return (int)(str1_len - str2_len);			// FIXME

		str1--;
		str2--;
		pos--;
	}

	return 0;
}

BOOK_TYPE reader_get_book_type(char* file_path)
{
	if(_strncmpr(".fb2", file_path, sizeof(".fb2")-1) == 0)
		return BOOK_TYPE_FB2;
	else if(_strncmpr(".fb2.zip", file_path, sizeof(".fb2.zip")-1) == 0)
		return BOOK_TYPE_FB2_ZIP;
	else
		return BOOK_TYPE_NONE;
}
