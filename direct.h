bool TryDirectConvert(char *command);
void WriteFontName(const char **buffpoint);
void InsertBasicStyle(const char *rtf, bool include_header_info);
void InsertHeaderStyle(char *command, bool include_header_info);
void InsertStyle(char *command, bool include_header_info);
