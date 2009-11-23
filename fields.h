#define FIELD_EQ       1
#define FIELD_REF      2
#define FIELD_SYMBOL   3 
#define FIELD_PAGE     4 
#define FIELD_PAGE_REF 5
#define FIELD_COMMENT  6

void startField(int type);
void endCurrentField(void);
void endAllFields(void);

void fprintfRTF_field_separator(void);
void set_field_separator(char c);

void set_fields_use_REF(int i);
int fields_use_REF(void);

void set_fields_use_EQ(int i);
int fields_use_EQ(void);

int processing_fields(void);
int EQ_field_active(void);
