void InsertBasicStyle(const char *rtf, bool include_header_info);
void InsertStyle(const char *command);

void SetCurrentStyle(const char *style);
char *GetCurrentStyle(void);
int IsSameAsCurrentStyle(const char *s);
void InsertCurrentStyle(void);

