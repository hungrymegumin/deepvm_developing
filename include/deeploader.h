/**
 * Filename:deeploader.h
 * Author:megumin
 * Date:4/10/2021
 */

#ifndef _DEEPLOADER_H
#define _DEEPLOADER_H

//
#define MAGIC_NUMBER 0x6d736100
#define VERSION 1

//
#define SECTION_TYPE_USER 0
#define SECTION_TYPE_TYPE 1
#define SECTION_TYPE_IMPORT 2
#define SECTION_TYPE_FUNC 3
#define SECTION_TYPE_TABLE 4
#define SECTION_TYPE_MEMORY 5
#define SECTION_TYPE_GLOBAL 6
#define SECTION_TYPE_EXPORT 7
#define SECTION_TYPE_START 8
#define SECTION_TYPE_ELEM 9
#define SECTION_TYPE_CODE 10
#define SECTION_TYPE_DATA 11

//
//type item
typedef struct DEEPType
{
    int param_count;
    int ret_count;
    char type[1];
} DEEPType;

//local variables item
typedef struct LocalVars
{
    int count;
    short local_type;
} LocalVars;

//function item
typedef struct DEEPFunction
{
    DEEPType *func_type; // the type of function
    LocalVars **localvars;
    int code_size;
    char *code_begin;
} DEEPFunction;

//
typedef struct DEEPExport
{
    char* name;
    int index;
    char tag;
};

/* Data structure of module, at present we only support 
two sections, which can make the program run*/
typedef struct DEEPModule
{
    int type_count;
    int function_count;
    int export_count;
    DEEPType **type_section;
    DEEPFunction **func_section;
    DEEPExport **export_section;
} DEEPModule;

//the difinition of listnode
typedef struct section_listnode
{
    char section_type;
    int section_size;
    char *section_begin;
    struct section_listnode *next;
} section_listnode;

DEEPModule* deep_load(char** p, int size);
int read_leb_u32(char** p);
#endif