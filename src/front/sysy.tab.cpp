/* A Bison parser, made by GNU Bison 3.5.1.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2020 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Undocumented macros, especially those whose name start with YY_,
   are private implementation details.  Do not rely on them.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.5.1"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 9 "src/front/sysy.y"

#include <memory>
#include <string>
#include <cstring>

#include "../../include/front/ast.hpp"
#include "../../include/front/variable.hpp"

int yylex();
void yyerror(std::unique_ptr<BaseAST> &ast,std::string s);

using namespace std;
//extern  yylval;  // 声明 yylval
extern Val_Table val_table;
extern Btype var_type;

extern int yylineno; //define and maintained in lex

#line 89 "src/front/sysy.tab.cpp"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Use api.header.include to #include this header
   instead of duplicating it here.  */
#ifndef YY_YY_SRC_FRONT_SYSY_TAB_HPP_INCLUDED
# define YY_YY_SRC_FRONT_SYSY_TAB_HPP_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif
/* "%code requires" blocks.  */
#line 1 "src/front/sysy.y"

  #include <memory>
  #include <string>
  #include "../../include/front/ast.hpp"
  #include "../../include/front/variable.hpp"
  #include <cstring>

#line 140 "src/front/sysy.tab.cpp"

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    INT = 258,
    VOID = 259,
    FLOAT = 260,
    RETURN = 261,
    CONST = 262,
    IF = 263,
    ELSE = 264,
    WHILE = 265,
    BREAK = 266,
    CONTINUE = 267,
    IDENT = 268,
    INT_CONST = 269,
    FLOAT_CONST = 270
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 32 "src/front/sysy.y"

  std::string *str_val;
  int int_val;
  float float_val;
  op op_val;
  Btype btype_val;
  BaseAST *ast_val;

#line 176 "src/front/sysy.tab.cpp"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (std::unique_ptr<BaseAST> &ast);

#endif /* !YY_YY_SRC_FRONT_SYSY_TAB_HPP_INCLUDED  */



#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))

/* Stored state numbers (used for stacks). */
typedef yytype_uint8 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && ! defined __ICC && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                            \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  15
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   263

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  35
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  50
/* YYNRULES -- Number of rules.  */
#define YYNRULES  110
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  190

#define YYUNDEFTOK  2
#define YYMAXUTOK   270


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    27,     2,     2,     2,    34,    26,     2,
      17,    18,    32,    30,    19,    31,     2,    33,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    16,
      29,    24,    28,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    20,     2,    21,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    22,    25,    23,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,    65,    65,    72,    77,    84,    92,    97,   106,   121,
     124,   130,   139,   147,   155,   166,   175,   181,   196,   199,
     202,   212,   216,   225,   237,   246,   251,   260,   265,   272,
     277,   284,   289,   296,   302,   306,   315,   319,   325,   331,
     340,   343,   349,   358,   370,   375,   389,   395,   408,   416,
     426,   431,   434,   442,   447,   462,   469,   476,   487,   497,
     507,   513,   522,   528,   536,   547,   553,   561,   569,   574,
     585,   595,   604,   612,   619,   627,   634,   642,   650,   658,
     666,   674,   682,   690,   697,   705,   713,   720,   728,   736,
     744,   752,   755,   767,   770,   773,   779,   782,   785,   788,
     814,   816,   823,   832,   841,   850,   863,   868,   878,   887,
     894
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "INT", "VOID", "FLOAT", "RETURN",
  "CONST", "IF", "ELSE", "WHILE", "BREAK", "CONTINUE", "IDENT",
  "INT_CONST", "FLOAT_CONST", "';'", "'('", "')'", "','", "'['", "']'",
  "'{'", "'}'", "'='", "'|'", "'&'", "'!'", "'>'", "'<'", "'+'", "'-'",
  "'*'", "'/'", "'%'", "$accept", "CompUnit", "DefUnitList", "DefUnits",
  "DefUnit", "FuncDef", "FuncFParamList", "FuncFParams", "FuncFParam",
  "FuncArrays", "FuncArray", "BasicType", "Block", "BlockItems",
  "BlockItem", "Stmt", "Else", "OptionExp", "Declarationlist",
  "ConstDeclList", "VarDeclList", "ConstDefs", "VarDefs", "ConstDef",
  "ConstInitVal", "ConstVals", "VarDef", "ArrayDefs", "ArrayDef",
  "Assignments", "Assignment", "InitVal", "InitVals", "ConstExp", "Exp",
  "LOrExp", "LAndExp", "EqExp", "RelExp", "AddExp", "MulExp", "UnaryExp",
  "UnaryOp", "PrimaryExp", "FuncRParamList", "FuncRParams", "Number",
  "LVal", "LArrays", "LArray", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_int16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,    59,    40,    41,    44,
      91,    93,   123,   125,    61,   124,    38,    33,    62,    60,
      43,    45,    42,    47,    37
};
# endif

#define YYPACT_NINF (-116)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
      86,  -116,  -116,  -116,   117,    11,  -116,    86,  -116,  -116,
      22,    26,  -116,  -116,    42,  -116,  -116,    -5,    58,  -116,
       3,  -116,  -116,     8,    75,  -116,    16,   117,   232,   178,
      84,   232,   178,   189,    42,   189,    82,    99,  -116,   126,
      -4,  -116,  -116,   232,  -116,  -116,  -116,   120,  -116,   118,
     116,    68,    53,    73,    94,  -116,   232,  -116,  -116,  -116,
     124,  -116,     7,  -116,  -116,    28,  -116,   127,  -116,   167,
    -116,  -116,  -116,  -116,   123,   117,   129,   130,  -116,   232,
     232,   133,  -116,   122,   131,   128,   132,   200,   208,   232,
     232,   232,   232,   232,  -116,   232,  -116,  -116,    34,  -116,
    -116,  -116,    57,   102,  -116,  -116,   144,   232,  -116,   135,
     153,   154,  -116,   232,   232,   232,   232,   232,    73,   232,
      73,    94,    94,  -116,  -116,  -116,   155,   178,  -116,   189,
    -116,   227,   157,   160,   169,   172,    47,  -116,    84,  -116,
     156,  -116,  -116,   180,   183,   185,  -116,  -116,    63,  -116,
     186,  -116,   232,  -116,   116,    68,    53,    53,    73,    73,
    -116,  -116,  -116,  -116,   194,   232,   232,  -116,  -116,   232,
    -116,  -116,  -116,  -116,  -116,   232,  -116,  -116,  -116,   210,
     211,  -116,  -116,    48,    48,   203,  -116,    48,  -116,  -116
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       0,    18,    19,    20,     0,     0,     2,     3,     5,     6,
       0,     0,    40,    41,     0,     1,     4,    56,    43,    46,
      57,    60,     7,     0,    42,    44,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     9,    12,     0,
     106,   104,   105,     0,    95,    94,    93,     0,    70,    71,
      73,    75,    78,    83,    86,    90,     0,    91,    97,    98,
     107,   109,     0,    55,    65,    56,    47,     0,    58,     0,
      48,    50,    45,    49,     0,     0,    13,    14,    16,   101,
       0,     0,    61,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    92,     0,    67,    68,     0,    59,
      52,    53,     0,    39,     8,    11,     0,     0,   103,     0,
     100,     0,    96,     0,     0,     0,     0,     0,    79,     0,
      80,    84,    85,    87,    88,    89,     0,     0,    66,     0,
      51,     0,     0,     0,     0,     0,   106,    22,     0,    31,
      39,    24,    26,     0,     0,     0,    62,    38,   107,    17,
       0,    99,     0,   110,    72,    74,    76,    77,    81,    82,
     108,    69,    54,    30,     0,     0,     0,    34,    35,     0,
      21,    23,    29,    25,    27,     0,    15,   102,    28,     0,
       0,    63,    64,    39,    39,    37,    33,    39,    32,    36
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -116,  -116,  -116,  -116,   219,  -116,  -116,  -116,   143,  -116,
    -116,    -2,   159,  -116,    96,  -115,  -116,  -116,   -94,  -116,
    -116,  -116,  -116,   214,   -30,  -116,   204,    -8,  -116,  -116,
    -116,   -31,  -116,   -21,   -29,  -116,   137,   138,    15,   -70,
      45,   -48,  -116,  -116,  -116,  -116,  -116,  -116,   -99,  -116
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     5,     6,     7,     8,     9,    36,    37,    38,    77,
      78,    10,   139,   140,   141,   142,   188,   143,    11,    12,
      13,    24,    18,    25,    70,   102,    19,    20,    21,   145,
     146,    63,    98,    71,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,   109,   110,    58,    59,    60,    61
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_uint8 yytable[] =
{
      64,    68,    14,    64,   148,    73,    26,    47,    94,   144,
      67,    15,    27,    79,    81,    28,    80,   118,   120,    29,
      40,    41,    42,    31,    43,    39,    26,    32,    28,    62,
      96,    97,    33,    64,    44,    17,    31,    45,    46,   101,
      35,   148,    22,   123,   124,   125,   144,   158,    28,   159,
     108,   111,    29,   127,   131,    23,   132,   128,   133,   134,
     135,   136,    41,    42,    79,    43,   126,    80,   185,   186,
     103,   169,   189,    39,   147,    44,   129,    30,    45,    46,
     130,    87,    88,    95,   148,   148,   150,   175,   148,     1,
       2,     3,    85,     4,    34,    86,   161,    65,    64,   162,
      74,   138,   164,    89,    90,     1,     2,     3,   131,     4,
     132,   147,   133,   134,   135,   136,    41,    42,    75,    43,
       1,     2,     3,   177,   103,   137,    91,    92,    93,    44,
     156,   157,    45,    46,   121,   122,   179,   180,   138,    76,
     181,    82,    84,    83,    95,   103,   182,   113,    99,   106,
     107,   112,   115,   151,   147,   147,   116,   114,   147,     1,
       2,     3,   131,     4,   132,   149,   133,   134,   135,   136,
      41,    42,   152,    43,   165,   153,   160,   166,   103,   170,
      40,    41,    42,    44,    43,   167,    45,    46,   168,    69,
     100,    40,    41,    42,    44,    43,   172,    45,    46,   173,
      62,   174,    40,    41,    42,    44,    43,   176,    45,    46,
     178,    69,   187,    40,    41,    42,    44,    43,   105,    45,
      46,    40,    41,    42,   117,    43,    16,    44,   183,   184,
      45,    46,   119,   104,    66,    44,   171,     0,    45,    46,
      40,    41,    42,   163,    43,    40,    41,    42,    72,    43,
     154,     0,   155,     0,    44,     0,     0,    45,    46,    44,
       0,     0,    45,    46
};

static const yytype_int16 yycheck[] =
{
      29,    32,     4,    32,   103,    35,    14,    28,    56,   103,
      31,     0,    17,    17,    43,    20,    20,    87,    88,    24,
      13,    14,    15,    20,    17,    27,    34,    24,    20,    22,
      23,    62,    24,    62,    27,    13,    20,    30,    31,    69,
      24,   140,    16,    91,    92,    93,   140,   117,    20,   119,
      79,    80,    24,    19,     6,    13,     8,    23,    10,    11,
      12,    13,    14,    15,    17,    17,    95,    20,   183,   184,
      22,    24,   187,    75,   103,    27,    19,    19,    30,    31,
      23,    28,    29,    20,   183,   184,   107,    24,   187,     3,
       4,     5,    24,     7,    19,    27,   127,    13,   127,   129,
      18,   103,   131,    30,    31,     3,     4,     5,     6,     7,
       8,   140,    10,    11,    12,    13,    14,    15,    19,    17,
       3,     4,     5,   152,    22,    23,    32,    33,    34,    27,
     115,   116,    30,    31,    89,    90,   165,   166,   140,    13,
     169,    21,    26,    25,    20,    22,   175,    25,    21,    20,
      20,    18,    24,    18,   183,   184,    24,    26,   187,     3,
       4,     5,     6,     7,     8,    21,    10,    11,    12,    13,
      14,    15,    19,    17,    17,    21,    21,    17,    22,    23,
      13,    14,    15,    27,    17,    16,    30,    31,    16,    22,
      23,    13,    14,    15,    27,    17,    16,    30,    31,    16,
      22,    16,    13,    14,    15,    27,    17,    21,    30,    31,
      16,    22,     9,    13,    14,    15,    27,    17,    75,    30,
      31,    13,    14,    15,    24,    17,     7,    27,    18,    18,
      30,    31,    24,    74,    30,    27,   140,    -1,    30,    31,
      13,    14,    15,    16,    17,    13,    14,    15,    34,    17,
     113,    -1,   114,    -1,    27,    -1,    -1,    30,    31,    27,
      -1,    -1,    30,    31
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,     3,     4,     5,     7,    36,    37,    38,    39,    40,
      46,    53,    54,    55,    46,     0,    39,    13,    57,    61,
      62,    63,    16,    13,    56,    58,    62,    17,    20,    24,
      19,    20,    24,    24,    19,    24,    41,    42,    43,    46,
      13,    14,    15,    17,    27,    30,    31,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    81,    82,
      83,    84,    22,    66,    69,    13,    61,    68,    66,    22,
      59,    68,    58,    59,    18,    19,    13,    44,    45,    17,
      20,    69,    21,    25,    26,    24,    27,    28,    29,    30,
      31,    32,    33,    34,    76,    20,    23,    66,    67,    21,
      23,    59,    60,    22,    47,    43,    20,    20,    69,    79,
      80,    69,    18,    25,    26,    24,    24,    24,    74,    24,
      74,    75,    75,    76,    76,    76,    69,    19,    23,    19,
      23,     6,     8,    10,    11,    12,    13,    23,    46,    47,
      48,    49,    50,    52,    53,    64,    65,    69,    83,    21,
      68,    18,    19,    21,    71,    72,    73,    73,    74,    74,
      21,    66,    59,    16,    69,    17,    17,    16,    16,    24,
      23,    49,    16,    16,    16,    24,    21,    69,    16,    69,
      69,    69,    69,    18,    18,    50,    50,     9,    51,    50
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_int8 yyr1[] =
{
       0,    35,    36,    37,    38,    38,    39,    39,    40,    41,
      41,    42,    42,    43,    43,    44,    44,    45,    46,    46,
      46,    47,    47,    48,    48,    49,    49,    50,    50,    50,
      50,    50,    50,    50,    50,    50,    51,    51,    52,    52,
      53,    53,    54,    55,    56,    56,    57,    57,    58,    58,
      59,    59,    59,    60,    60,    61,    61,    61,    61,    62,
      62,    63,    64,    65,    65,    66,    66,    66,    67,    67,
      68,    69,    70,    70,    71,    71,    72,    72,    72,    73,
      73,    73,    73,    73,    74,    74,    74,    75,    75,    75,
      75,    76,    76,    77,    77,    77,    78,    78,    78,    78,
      79,    79,    80,    80,    81,    81,    82,    82,    83,    83,
      84
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     1,     2,     1,     1,     2,     6,     1,
       0,     3,     1,     2,     2,     4,     1,     3,     1,     1,
       1,     3,     2,     2,     1,     2,     1,     2,     3,     2,
       2,     1,     6,     5,     2,     2,     2,     0,     1,     0,
       1,     1,     3,     2,     1,     3,     1,     3,     3,     3,
       1,     3,     2,     1,     3,     3,     1,     1,     3,     4,
       1,     4,     1,     3,     3,     1,     3,     2,     1,     3,
       1,     1,     4,     1,     4,     1,     4,     4,     1,     3,
       3,     4,     4,     1,     3,     3,     1,     3,     3,     3,
       1,     1,     2,     1,     1,     1,     3,     1,     1,     4,
       1,     0,     3,     1,     1,     1,     1,     1,     4,     1,
       4
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (ast, YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256



/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)

/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value, ast); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo, int yytype, YYSTYPE const * const yyvaluep, std::unique_ptr<BaseAST> &ast)
{
  FILE *yyoutput = yyo;
  YYUSE (yyoutput);
  YYUSE (ast);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyo, yytoknum[yytype], *yyvaluep);
# endif
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo, int yytype, YYSTYPE const * const yyvaluep, std::unique_ptr<BaseAST> &ast)
{
  YYFPRINTF (yyo, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyo, yytype, yyvaluep, ast);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp, int yyrule, std::unique_ptr<BaseAST> &ast)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[+yyssp[yyi + 1 - yynrhs]],
                       &yyvsp[(yyi + 1) - (yynrhs)]
                                              , ast);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule, ast); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen(S) (YY_CAST (YYPTRDIFF_T, strlen (S)))
#  else
/* Return the length of YYSTR.  */
static YYPTRDIFF_T
yystrlen (const char *yystr)
{
  YYPTRDIFF_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYPTRDIFF_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYPTRDIFF_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            else
              goto append;

          append:
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (yyres)
    return yystpcpy (yyres, yystr) - yyres;
  else
    return yystrlen (yystr);
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYPTRDIFF_T *yymsg_alloc, char **yymsg,
                yy_state_t *yyssp, int yytoken)
{
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat: reported tokens (one for the "unexpected",
     one per "expected"). */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Actual size of YYARG. */
  int yycount = 0;
  /* Cumulated lengths of YYARG.  */
  YYPTRDIFF_T yysize = 0;

  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[+*yyssp];
      YYPTRDIFF_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
      yysize = yysize0;
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYPTRDIFF_T yysize1
                    = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
                    yysize = yysize1;
                  else
                    return 2;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
    default: /* Avoid compiler warnings. */
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    /* Don't count the "%s"s in the final size, but reserve room for
       the terminator.  */
    YYPTRDIFF_T yysize1 = yysize + (yystrlen (yyformat) - 2 * yycount) + 1;
    if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
      yysize = yysize1;
    else
      return 2;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          ++yyp;
          ++yyformat;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, std::unique_ptr<BaseAST> &ast)
{
  YYUSE (yyvaluep);
  YYUSE (ast);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;


/*----------.
| yyparse.  |
`----------*/

int
yyparse (std::unique_ptr<BaseAST> &ast)
{
    yy_state_fast_t yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss;
    yy_state_t *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYPTRDIFF_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYPTRDIFF_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    goto yyexhaustedlab;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
# undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 2:
#line 65 "src/front/sysy.y"
              {
   auto comp_unit = make_unique<CompUnitAST>();
   comp_unit->start = unique_ptr<BaseAST>((yyvsp[0].ast_val));
   ast = std::move(comp_unit);
 }
#line 1508 "src/front/sysy.tab.cpp"
    break;

  case 3:
#line 72 "src/front/sysy.y"
           {
    (yyval.ast_val) = (yyvsp[0].ast_val);
  }
#line 1516 "src/front/sysy.tab.cpp"
    break;

  case 4:
#line 77 "src/front/sysy.y"
                    {
    DefUnitsAST* ast = dynamic_cast<DefUnitsAST*>((yyvsp[-1].ast_val));
    if(ast==nullptr){
      std::cerr << "Exception: " << "dynamic_cast failed"<< std::endl;
    }
    ast->unit_list.push_back(unique_ptr<BaseAST>((yyvsp[0].ast_val)));
    (yyval.ast_val) = ast;
  }
#line 1529 "src/front/sysy.tab.cpp"
    break;

  case 5:
#line 84 "src/front/sysy.y"
            {
    auto ast = new DefUnitsAST();
    ast->unit_list.push_back(unique_ptr<BaseAST>((yyvsp[0].ast_val)));
    (yyval.ast_val) = ast;
  }
#line 1539 "src/front/sysy.tab.cpp"
    break;

  case 6:
#line 92 "src/front/sysy.y"
            {
    auto ast = new DefUnitAST();
    ast->content = unique_ptr<BaseAST>((yyvsp[0].ast_val));
    (yyval.ast_val)=ast;
  }
#line 1549 "src/front/sysy.tab.cpp"
    break;

  case 7:
#line 97 "src/front/sysy.y"
                     {
    auto ast = new DefUnitAST();
    ast->content = unique_ptr<BaseAST>((yyvsp[-1].ast_val));
    (yyval.ast_val)=ast;
  }
#line 1559 "src/front/sysy.tab.cpp"
    break;

  case 8:
#line 106 "src/front/sysy.y"
                                                 {
    auto ast=new FuncDefAST();
    auto typeast=new DeclarationTypeAST();
    typeast->type=(yyvsp[-5].btype_val);
    ast->type = unique_ptr<BaseAST>(typeast);
    ast->ident = *unique_ptr<string>((yyvsp[-4].str_val));
    ast->params=unique_ptr<BaseAST>((yyvsp[-2].ast_val));
    ast->block = unique_ptr<BaseAST>((yyvsp[0].ast_val));
    (yyval.ast_val)=ast;
  }
#line 1574 "src/front/sysy.tab.cpp"
    break;

  case 9:
#line 121 "src/front/sysy.y"
              {
    (yyval.ast_val) = (yyvsp[0].ast_val);
  }
#line 1582 "src/front/sysy.tab.cpp"
    break;

  case 10:
#line 124 "src/front/sysy.y"
  {
    (yyval.ast_val) = nullptr;
  }
#line 1590 "src/front/sysy.tab.cpp"
    break;

  case 11:
#line 130 "src/front/sysy.y"
                             {
    FuncFParamsAST* ast = nullptr;
    ast = dynamic_cast<FuncFParamsAST*>((yyvsp[-2].ast_val));
    if(ast==nullptr){
      std::cerr << "Exception: " << "dynamic_cast failed"<< std::endl;
    }
    ast->paramlist.push_back(unique_ptr<BaseAST>((yyvsp[0].ast_val)));
    (yyval.ast_val) = ast;
  }
#line 1604 "src/front/sysy.tab.cpp"
    break;

  case 12:
#line 139 "src/front/sysy.y"
             {
    auto ast = new FuncFParamsAST();
    ast->paramlist.push_back(unique_ptr<BaseAST>((yyvsp[0].ast_val)));
    (yyval.ast_val) = ast;
  }
#line 1614 "src/front/sysy.tab.cpp"
    break;

  case 13:
#line 147 "src/front/sysy.y"
                    {
      auto ast = new FuncFParamAST();
      ast->type = (yyvsp[-1].btype_val);
      var_type=(yyvsp[-1].btype_val);
      ast->ident = *((yyvsp[0].str_val));
      ast->arraydef = nullptr;
      (yyval.ast_val) = ast;
    }
#line 1627 "src/front/sysy.tab.cpp"
    break;

  case 14:
#line 155 "src/front/sysy.y"
                         {
      auto ast = new FuncFParamAST();
      var_type=(yyvsp[-1].btype_val);
      ast->type = (yyvsp[-1].btype_val);
      ast->ident = "";
      ast->arraydef = unique_ptr<BaseAST>((yyvsp[0].ast_val));
      (yyval.ast_val) = ast;
    }
#line 1640 "src/front/sysy.tab.cpp"
    break;

  case 15:
#line 166 "src/front/sysy.y"
                             {
    FuncArrayAST* ast = nullptr;
    ast = dynamic_cast<FuncArrayAST*>((yyvsp[-3].ast_val));
    if(ast==nullptr){
      throw std::runtime_error("error: FuncArrayAST dynamic_cast failed");
    }
    ast->dimon_list.push_back(unique_ptr<BaseAST>((yyvsp[-1].ast_val)));
    (yyval.ast_val) = ast;
  }
#line 1654 "src/front/sysy.tab.cpp"
    break;

  case 16:
#line 175 "src/front/sysy.y"
            {
    (yyval.ast_val) = (yyvsp[0].ast_val);
  }
#line 1662 "src/front/sysy.tab.cpp"
    break;

  case 17:
#line 181 "src/front/sysy.y"
               {
    auto ast = new FuncArrayAST();
    ast->ident = *((yyvsp[-2].str_val));
    (yyval.ast_val) = ast;
  }
#line 1672 "src/front/sysy.tab.cpp"
    break;

  case 18:
#line 196 "src/front/sysy.y"
               {
  (yyval.btype_val) = Btype::BINT;
}
#line 1680 "src/front/sysy.tab.cpp"
    break;

  case 19:
#line 199 "src/front/sysy.y"
        {
    (yyval.btype_val) = Btype::BVOID;
}
#line 1688 "src/front/sysy.tab.cpp"
    break;

  case 20:
#line 202 "src/front/sysy.y"
         {
    (yyval.btype_val) = Btype::BFLOAT;
}
#line 1696 "src/front/sysy.tab.cpp"
    break;

  case 21:
#line 212 "src/front/sysy.y"
                       {
    auto ast=new BlockAST();
    ast->blockitems = unique_ptr<BaseAST>((yyvsp[-1].ast_val));
    (yyval.ast_val)=ast;
  }
#line 1706 "src/front/sysy.tab.cpp"
    break;

  case 22:
#line 216 "src/front/sysy.y"
           {
    auto ast=new BlockAST();
    ast->blockitems=nullptr;
    (yyval.ast_val)=ast;
  }
#line 1716 "src/front/sysy.tab.cpp"
    break;

  case 23:
#line 225 "src/front/sysy.y"
                       {
    BlockItemsAST* Items = nullptr;

      Items = dynamic_cast<BlockItemsAST*>((yyvsp[-1].ast_val));
      //后续可能会改进
      if(Items==nullptr){
        std::cerr << "Exception: " << "dynamic_cast failed"<< std::endl;
      }

    Items->itemlist.push_back(unique_ptr<BaseAST>((yyvsp[0].ast_val)));
    (yyval.ast_val) = Items;
  }
#line 1733 "src/front/sysy.tab.cpp"
    break;

  case 24:
#line 237 "src/front/sysy.y"
              {
      auto Items = new BlockItemsAST();
      Items->itemlist.push_back(unique_ptr<BaseAST>((yyvsp[0].ast_val)));
      (yyval.ast_val) = Items;
    }
#line 1743 "src/front/sysy.tab.cpp"
    break;

  case 25:
#line 246 "src/front/sysy.y"
                     {
    auto ast = new BlockItemAST();
    ast->content = unique_ptr<BaseAST>((yyvsp[-1].ast_val));
    (yyval.ast_val)=ast;
  }
#line 1753 "src/front/sysy.tab.cpp"
    break;

  case 26:
#line 251 "src/front/sysy.y"
       {
    auto ast = new StmtAST();
    ast->sent = unique_ptr<BaseAST>((yyvsp[0].ast_val));
    (yyval.ast_val) = ast;
  }
#line 1763 "src/front/sysy.tab.cpp"
    break;

  case 27:
#line 260 "src/front/sysy.y"
                  {
    auto ast =new BlockItemAST();
    ast->content = unique_ptr<BaseAST>((yyvsp[-1].ast_val));
    (yyval.ast_val)=ast;
  }
#line 1773 "src/front/sysy.tab.cpp"
    break;

  case 28:
#line 265 "src/front/sysy.y"
                 {
    auto ast = new BlockItemAST();
    auto retAst=new ReturnAST();
    retAst->retNum=unique_ptr<BaseAST>((yyvsp[-1].ast_val));
    ast->content=unique_ptr<BaseAST>(retAst);
    (yyval.ast_val)=ast;
  }
#line 1785 "src/front/sysy.tab.cpp"
    break;

  case 29:
#line 272 "src/front/sysy.y"
                {
    auto ast = new BlockItemAST();
    ast->content = unique_ptr<BaseAST>((yyvsp[-1].ast_val));
    (yyval.ast_val)=ast;
  }
#line 1795 "src/front/sysy.tab.cpp"
    break;

  case 30:
#line 277 "src/front/sysy.y"
              {
    auto ast = new BlockItemAST();
    auto retAst=new ReturnAST();
    retAst->retNum=nullptr;
    ast->content=unique_ptr<BaseAST>(retAst);
    (yyval.ast_val)=ast;
  }
#line 1807 "src/front/sysy.tab.cpp"
    break;

  case 31:
#line 284 "src/front/sysy.y"
       {
    auto ast = new BlockItemAST();
    ast->content = unique_ptr<BaseAST>((yyvsp[0].ast_val));
    (yyval.ast_val)=ast;
  }
#line 1817 "src/front/sysy.tab.cpp"
    break;

  case 32:
#line 289 "src/front/sysy.y"
                          {
    auto ast=new IfElseAST();
    ast->exp=unique_ptr<BaseAST>((yyvsp[-3].ast_val));
    ast->then_part=unique_ptr<BaseAST>((yyvsp[-1].ast_val));
    ast->else_part=unique_ptr<BaseAST>((yyvsp[0].ast_val));
    (yyval.ast_val)=ast; 
  }
#line 1829 "src/front/sysy.tab.cpp"
    break;

  case 33:
#line 296 "src/front/sysy.y"
                        {
    auto ast=new WhileAST();
    ast->exp=unique_ptr<BaseAST>((yyvsp[-2].ast_val));
    ast->body=unique_ptr<BaseAST>((yyvsp[0].ast_val));
    (yyval.ast_val)=ast;
  }
#line 1840 "src/front/sysy.tab.cpp"
    break;

  case 34:
#line 302 "src/front/sysy.y"
           {
    auto ast=new BreakAST();
    (yyval.ast_val)=ast;
  }
#line 1849 "src/front/sysy.tab.cpp"
    break;

  case 35:
#line 306 "src/front/sysy.y"
              {
    auto ast=new ContinueAST();
    (yyval.ast_val)=ast;
  }
#line 1858 "src/front/sysy.tab.cpp"
    break;

  case 36:
#line 315 "src/front/sysy.y"
           {
    (yyval.ast_val)=(yyvsp[0].ast_val);
  }
#line 1866 "src/front/sysy.tab.cpp"
    break;

  case 37:
#line 319 "src/front/sysy.y"
  {
    (yyval.ast_val)=nullptr;
  }
#line 1874 "src/front/sysy.tab.cpp"
    break;

  case 38:
#line 325 "src/front/sysy.y"
       {
      auto ast=new OptionExpAST();
      ast->exp=unique_ptr<BaseAST>((yyvsp[0].ast_val));
      (yyval.ast_val)=ast;
    }
#line 1884 "src/front/sysy.tab.cpp"
    break;

  case 39:
#line 331 "src/front/sysy.y"
    {
      auto ast=new OptionExpAST();
      ast->exp=nullptr;
      (yyval.ast_val) = ast;
    }
#line 1894 "src/front/sysy.tab.cpp"
    break;

  case 40:
#line 340 "src/front/sysy.y"
                {
    (yyval.ast_val) = (yyvsp[0].ast_val);
  }
#line 1902 "src/front/sysy.tab.cpp"
    break;

  case 41:
#line 343 "src/front/sysy.y"
              {
    (yyval.ast_val) = (yyvsp[0].ast_val);
  }
#line 1910 "src/front/sysy.tab.cpp"
    break;

  case 42:
#line 349 "src/front/sysy.y"
                            {
    auto constdecllist = new ConstDeclListAST();
    constdecllist->type = (yyvsp[-1].btype_val);
    var_type=(yyvsp[-1].btype_val);
    constdecllist->constdefs = unique_ptr<BaseAST>((yyvsp[0].ast_val));
    (yyval.ast_val) = constdecllist;
  }
#line 1922 "src/front/sysy.tab.cpp"
    break;

  case 43:
#line 358 "src/front/sysy.y"
                    {
    auto vardecllist = new VarDeclListAST();
    //std::cout << "var_type initial: " << var_type << std::endl;
    //std::cout << "var_type after assign: " << var_type << std::endl;
    vardecllist->type = (yyvsp[-1].btype_val);
    vardecllist->vardefs = unique_ptr<BaseAST>((yyvsp[0].ast_val));
    //std::cout << "VarDeclList type: " << vardecllist->type << std::endl;  // 输出 vardecllist->type 的值
    (yyval.ast_val) = vardecllist;
  }
#line 1936 "src/front/sysy.tab.cpp"
    break;

  case 44:
#line 370 "src/front/sysy.y"
           {
    auto Defs = new ConstDefsAST();
    Defs->constdefs.push_back(unique_ptr<BaseAST>((yyvsp[0].ast_val)));
    (yyval.ast_val) = Defs;
  }
#line 1946 "src/front/sysy.tab.cpp"
    break;

  case 45:
#line 375 "src/front/sysy.y"
                         {
    ConstDefsAST* defsast = nullptr;
    defsast = dynamic_cast<ConstDefsAST*>((yyvsp[-2].ast_val));
    //后续可能会改进
    if(defsast==nullptr){
        std::cerr << "Exception: " << "dynamic_cast failed"<< std::endl;
      }
    defsast->constdefs.push_back(unique_ptr<BaseAST>((yyvsp[0].ast_val)));
    (yyval.ast_val) = defsast;
  }
#line 1961 "src/front/sysy.tab.cpp"
    break;

  case 46:
#line 389 "src/front/sysy.y"
         {
    auto Defs = new VarDefsAST();
    Defs->vardefs.push_back(unique_ptr<BaseAST>((yyvsp[0].ast_val)));
    //std::cout <<"VarDefs"<<Defs->type << std::endl; 
    (yyval.ast_val) = Defs;
  }
#line 1972 "src/front/sysy.tab.cpp"
    break;

  case 47:
#line 395 "src/front/sysy.y"
                     {
    VarDefsAST* defsast = nullptr;
    defsast = dynamic_cast<VarDefsAST*>((yyvsp[-2].ast_val));
    //后续可能会改进
    if(defsast==nullptr){
        std::cerr << "Exception: " << "dynamic_cast failed"<< std::endl;
      }
    defsast->vardefs.push_back(unique_ptr<BaseAST>((yyvsp[0].ast_val)));
    (yyval.ast_val) = defsast;
  }
#line 1987 "src/front/sysy.tab.cpp"
    break;

  case 48:
#line 408 "src/front/sysy.y"
                         {
    auto ast = new ConstDefAST();
    ast->ident = *((yyvsp[-2].str_val));
    ast->arraydef = nullptr;
    ast->initval = unique_ptr<BaseAST>((yyvsp[0].ast_val));
    (yyval.ast_val) = ast;
  }
#line 1999 "src/front/sysy.tab.cpp"
    break;

  case 49:
#line 416 "src/front/sysy.y"
                             {
    auto ast = new ConstDefAST();
    ast->ident = "";
    ast->arraydef = unique_ptr<BaseAST>((yyvsp[-2].ast_val));
    ast->initval = unique_ptr<BaseAST>((yyvsp[0].ast_val));
    (yyval.ast_val) = ast;
  }
#line 2011 "src/front/sysy.tab.cpp"
    break;

  case 50:
#line 426 "src/front/sysy.y"
           {
    auto initval = new ConstExpAST();
    initval->exp = unique_ptr<BaseAST>((yyvsp[0].ast_val));
    (yyval.ast_val) = initval;
  }
#line 2021 "src/front/sysy.tab.cpp"
    break;

  case 51:
#line 431 "src/front/sysy.y"
                    {
    (yyval.ast_val)=(yyvsp[-1].ast_val);
  }
#line 2029 "src/front/sysy.tab.cpp"
    break;

  case 52:
#line 434 "src/front/sysy.y"
         {
    auto initvals = new ConstInitValsAST();
    initvals->constexp = nullptr;
    (yyval.ast_val) = initvals;
  }
#line 2039 "src/front/sysy.tab.cpp"
    break;

  case 53:
#line 442 "src/front/sysy.y"
               {
    auto ast = new ConstInitValsAST();
    ast->array_val_list.push_back(unique_ptr<BaseAST>((yyvsp[0].ast_val)));
    (yyval.ast_val) = ast;
  }
#line 2049 "src/front/sysy.tab.cpp"
    break;

  case 54:
#line 447 "src/front/sysy.y"
                             {
    ConstInitValsAST* valsast = nullptr;
    valsast = dynamic_cast<ConstInitValsAST*>((yyvsp[-2].ast_val));
    if(valsast==nullptr){
        throw std::runtime_error("error: ConstInitValsAST dynamic_cast failed");
      }
    valsast->array_val_list.push_back(unique_ptr<BaseAST>((yyvsp[0].ast_val)));
    (yyval.ast_val) = valsast;
  }
#line 2063 "src/front/sysy.tab.cpp"
    break;

  case 55:
#line 462 "src/front/sysy.y"
                    {
    auto ast = new VarDefAST();
    ast->ident = *((yyvsp[-2].str_val));
    ast->arraydef = nullptr;
    ast->initval = unique_ptr<BaseAST>((yyvsp[0].ast_val));
    (yyval.ast_val) = ast;
  }
#line 2075 "src/front/sysy.tab.cpp"
    break;

  case 56:
#line 469 "src/front/sysy.y"
         {
    auto ast = new VarDefAST();
    ast->ident = *((yyvsp[0].str_val));
    ast->arraydef = nullptr;
    ast->initval = nullptr;
    (yyval.ast_val) = ast;
  }
#line 2087 "src/front/sysy.tab.cpp"
    break;

  case 57:
#line 476 "src/front/sysy.y"
            {
    auto ast = new VarDefAST();
    ast->ident = "";
    ast->arraydef = unique_ptr<BaseAST>((yyvsp[0].ast_val));
    auto initvals = new InitValsAST();
    initvals->exp = nullptr;
    initvals->is_undef= true ;
    ast->initval = unique_ptr<BaseAST>(initvals);
    (yyval.ast_val) = ast;
  }
#line 2102 "src/front/sysy.tab.cpp"
    break;

  case 58:
#line 487 "src/front/sysy.y"
                         {
    auto ast = new VarDefAST();
    ast->ident = "";
    ast->arraydef = unique_ptr<BaseAST>((yyvsp[-2].ast_val));
    ast->initval = unique_ptr<BaseAST>((yyvsp[0].ast_val));
    (yyval.ast_val) = ast;
  }
#line 2114 "src/front/sysy.tab.cpp"
    break;

  case 59:
#line 497 "src/front/sysy.y"
                            {
    ArrayDefsAST* ast = nullptr;
    ast = dynamic_cast<ArrayDefsAST*>((yyvsp[-3].ast_val));
    if(ast==nullptr){
      throw std::runtime_error("error: ArrayDefsAST cast failed");
    }
    ast->dimon_list.push_back(unique_ptr<BaseAST>((yyvsp[-1].ast_val))); 
    (yyval.ast_val) = ast;
  }
#line 2128 "src/front/sysy.tab.cpp"
    break;

  case 60:
#line 507 "src/front/sysy.y"
           {
    (yyval.ast_val) = (yyvsp[0].ast_val);
  }
#line 2136 "src/front/sysy.tab.cpp"
    break;

  case 61:
#line 513 "src/front/sysy.y"
                         {
    auto ast = new ArrayDefsAST();
    ast->ident = *((yyvsp[-3].str_val));
    ast->dimon_list.push_back(unique_ptr<BaseAST>((yyvsp[-1].ast_val)));
    (yyval.ast_val) = ast;
  }
#line 2147 "src/front/sysy.tab.cpp"
    break;

  case 62:
#line 522 "src/front/sysy.y"
               {
    (yyval.ast_val)=(yyvsp[0].ast_val);
  }
#line 2155 "src/front/sysy.tab.cpp"
    break;

  case 63:
#line 528 "src/front/sysy.y"
                {
    auto ast=new AssignAST();
    ast->ident=*((yyvsp[-2].str_val));
    ast->arraydef=nullptr;
    ast->exp=unique_ptr<BaseAST>((yyvsp[0].ast_val));
    //set_sym_val(sym_head,sym_tail,*($1),$3);
    (yyval.ast_val)=ast;
  }
#line 2168 "src/front/sysy.tab.cpp"
    break;

  case 64:
#line 536 "src/front/sysy.y"
                 {
    auto ast=new AssignAST();
    ast->ident="";
    ast->arraydef=unique_ptr<BaseAST>((yyvsp[-2].ast_val));
    ast->exp=unique_ptr<BaseAST>((yyvsp[0].ast_val));
    (yyval.ast_val)=ast;
  }
#line 2180 "src/front/sysy.tab.cpp"
    break;

  case 65:
#line 547 "src/front/sysy.y"
      {
    auto initval = new ExpAST();
    initval->exp = unique_ptr<BaseAST>((yyvsp[0].ast_val));
    (yyval.ast_val) = initval;
  }
#line 2190 "src/front/sysy.tab.cpp"
    break;

  case 66:
#line 553 "src/front/sysy.y"
                  {
    InitValsAST* ast=dynamic_cast<InitValsAST*>((yyvsp[-1].ast_val));
    if(ast==nullptr){
      throw std::runtime_error("error: ArrayDefsAST cast failed");
    }
    (yyval.ast_val) = ast;
  }
#line 2202 "src/front/sysy.tab.cpp"
    break;

  case 67:
#line 561 "src/front/sysy.y"
         {
    auto initvals = new InitValsAST();
    initvals->exp = nullptr;
    (yyval.ast_val) = initvals;
  }
#line 2212 "src/front/sysy.tab.cpp"
    break;

  case 68:
#line 569 "src/front/sysy.y"
          {
    auto ast = new InitValsAST();
    ast->array_val_list.push_back(unique_ptr<BaseAST>((yyvsp[0].ast_val)));
    (yyval.ast_val) = ast;
  }
#line 2222 "src/front/sysy.tab.cpp"
    break;

  case 69:
#line 574 "src/front/sysy.y"
                       {
    InitValsAST* valsast = nullptr;
    valsast = dynamic_cast<InitValsAST*>((yyvsp[-2].ast_val));
    if(valsast==nullptr){
        std::cerr << "Exception: " << "dynamic_cast failed"<< std::endl;
      }
    valsast->array_val_list.push_back(unique_ptr<BaseAST>((yyvsp[0].ast_val)));
    (yyval.ast_val) = valsast;
  }
#line 2236 "src/front/sysy.tab.cpp"
    break;

  case 70:
#line 585 "src/front/sysy.y"
     {
    auto ast=new ConstExpAST();
    ast->exp=unique_ptr<BaseAST>((yyvsp[0].ast_val));
    (yyval.ast_val)=ast;
  }
#line 2246 "src/front/sysy.tab.cpp"
    break;

  case 71:
#line 595 "src/front/sysy.y"
           {
    auto ast=new ExpAST();
    ast->exp=unique_ptr<BaseAST>((yyvsp[0].ast_val));
    (yyval.ast_val)=ast;
  }
#line 2256 "src/front/sysy.tab.cpp"
    break;

  case 72:
#line 604 "src/front/sysy.y"
                        {
    auto lorexp = new BinaryExpAST();
    lorexp->exp1 = unique_ptr<BaseAST>((yyvsp[-3].ast_val));
    lorexp->exp2 = unique_ptr<BaseAST>((yyvsp[0].ast_val));
    lorexp->op2=op::OR;
    //lorexp->calc_f=($1->calc_f)&&($4->calc_f);
    (yyval.ast_val) = lorexp;
  }
#line 2269 "src/front/sysy.tab.cpp"
    break;

  case 73:
#line 612 "src/front/sysy.y"
            {
    (yyval.ast_val)=(yyvsp[0].ast_val);
  }
#line 2277 "src/front/sysy.tab.cpp"
    break;

  case 74:
#line 619 "src/front/sysy.y"
                       {
    auto landexp = new BinaryExpAST();
    landexp->exp1 = unique_ptr<BaseAST>((yyvsp[-3].ast_val));
    landexp->exp2 = unique_ptr<BaseAST>((yyvsp[0].ast_val));
    landexp->op2 = op::AND;
    //landexp->calc_f=($1->calc_f)&&($4->calc_f);
    (yyval.ast_val) = landexp;
  }
#line 2290 "src/front/sysy.tab.cpp"
    break;

  case 75:
#line 627 "src/front/sysy.y"
          {
    (yyval.ast_val)=(yyvsp[0].ast_val);
  }
#line 2298 "src/front/sysy.tab.cpp"
    break;

  case 76:
#line 634 "src/front/sysy.y"
                      {
    auto eqexp = new BinaryExpAST();
    eqexp->exp1 = unique_ptr<BaseAST>((yyvsp[-3].ast_val));
    eqexp->exp2 = unique_ptr<BaseAST>((yyvsp[0].ast_val));
    eqexp->op2 = op::EQ;
    //eqexp->calc_f=($1->calc_f)&&($4->calc_f);
    (yyval.ast_val) = eqexp;
  }
#line 2311 "src/front/sysy.tab.cpp"
    break;

  case 77:
#line 642 "src/front/sysy.y"
                         {
    auto eqexp = new BinaryExpAST();
    eqexp->exp1 = unique_ptr<BaseAST>((yyvsp[-3].ast_val));
    eqexp->exp2 = unique_ptr<BaseAST>((yyvsp[0].ast_val));
    eqexp->op2 = op::NE;
    //eqexp->calc_f=($1->calc_f)&&($4->calc_f);
    (yyval.ast_val) = eqexp;
  }
#line 2324 "src/front/sysy.tab.cpp"
    break;

  case 78:
#line 650 "src/front/sysy.y"
           {
    (yyval.ast_val)=(yyvsp[0].ast_val);
  }
#line 2332 "src/front/sysy.tab.cpp"
    break;

  case 79:
#line 658 "src/front/sysy.y"
                    {
    auto relexp = new BinaryExpAST();
    relexp->exp1 = unique_ptr<BaseAST>((yyvsp[-2].ast_val));
    relexp->exp2 = unique_ptr<BaseAST>((yyvsp[0].ast_val));
    relexp->op2 = op::GT;
    //relexp->calc_f=($1->calc_f)&&($3->calc_f);
    (yyval.ast_val) = relexp;
  }
#line 2345 "src/front/sysy.tab.cpp"
    break;

  case 80:
#line 666 "src/front/sysy.y"
                      {
    auto relexp = new BinaryExpAST();
    relexp->exp1 = unique_ptr<BaseAST>((yyvsp[-2].ast_val));
    relexp->exp2 = unique_ptr<BaseAST>((yyvsp[0].ast_val));
    relexp->op2 = op::LT;
    //relexp->calc_f=($1->calc_f)&&($3->calc_f);
    (yyval.ast_val) = relexp;
  }
#line 2358 "src/front/sysy.tab.cpp"
    break;

  case 81:
#line 674 "src/front/sysy.y"
                         {
    auto relexp = new BinaryExpAST();
    relexp->exp1 = unique_ptr<BaseAST>((yyvsp[-3].ast_val));
    relexp->exp2 = unique_ptr<BaseAST>((yyvsp[0].ast_val));
    relexp->op2 = op::GE;
    //relexp->calc_f=($1->calc_f)&&($4->calc_f);
    (yyval.ast_val) = relexp;
  }
#line 2371 "src/front/sysy.tab.cpp"
    break;

  case 82:
#line 682 "src/front/sysy.y"
                         {
    auto relexp = new BinaryExpAST();
    relexp->exp1 = unique_ptr<BaseAST>((yyvsp[-3].ast_val));
    relexp->exp2 = unique_ptr<BaseAST>((yyvsp[0].ast_val));
    relexp->op2 = op::LE;
    //relexp->calc_f=($1->calc_f)&&($4->calc_f);
    (yyval.ast_val) = relexp;
  }
#line 2384 "src/front/sysy.tab.cpp"
    break;

  case 83:
#line 690 "src/front/sysy.y"
           {
    (yyval.ast_val)=(yyvsp[0].ast_val);
  }
#line 2392 "src/front/sysy.tab.cpp"
    break;

  case 84:
#line 697 "src/front/sysy.y"
                    {
    auto addexp = new BinaryExpAST();
    addexp->exp1 = unique_ptr<BaseAST>((yyvsp[-2].ast_val));
    addexp->exp2 = unique_ptr<BaseAST>((yyvsp[0].ast_val));
    addexp->op2 = op::ADD;
    //addexp->calc_f=($1->calc_f)&&($3->calc_f);
    (yyval.ast_val) = addexp;
  }
#line 2405 "src/front/sysy.tab.cpp"
    break;

  case 85:
#line 705 "src/front/sysy.y"
                      {
    auto addexp = new BinaryExpAST();
    addexp->exp1 = unique_ptr<BaseAST>((yyvsp[-2].ast_val));
    addexp->exp2 = unique_ptr<BaseAST>((yyvsp[0].ast_val));
    addexp->op2 = op::SUB;
    //addexp->calc_f=($1->calc_f)&&($3->calc_f);
    (yyval.ast_val) = addexp;
  }
#line 2418 "src/front/sysy.tab.cpp"
    break;

  case 86:
#line 713 "src/front/sysy.y"
           {
    (yyval.ast_val)=(yyvsp[0].ast_val);
  }
#line 2426 "src/front/sysy.tab.cpp"
    break;

  case 87:
#line 720 "src/front/sysy.y"
                      {
    auto mulexp = new BinaryExpAST();
    mulexp->exp1 = unique_ptr<BaseAST>((yyvsp[-2].ast_val));
    mulexp->op2 = op::MUL;
    mulexp->exp2 = unique_ptr<BaseAST>((yyvsp[0].ast_val));
    //mulexp->calc_f=($1->calc_f)&&($3->calc_f);
    (yyval.ast_val) = mulexp;
  }
#line 2439 "src/front/sysy.tab.cpp"
    break;

  case 88:
#line 728 "src/front/sysy.y"
                        {
    auto mulexp = new BinaryExpAST();
    mulexp->exp1 = unique_ptr<BaseAST>((yyvsp[-2].ast_val));
    mulexp->op2 = op::DIV;
    mulexp->exp2 = unique_ptr<BaseAST>((yyvsp[0].ast_val));
    //mulexp->calc_f=($1->calc_f)&&($3->calc_f);
    (yyval.ast_val) = mulexp;
  }
#line 2452 "src/front/sysy.tab.cpp"
    break;

  case 89:
#line 736 "src/front/sysy.y"
                        {
    auto mulexp = new BinaryExpAST();
    mulexp->exp1 = unique_ptr<BaseAST>((yyvsp[-2].ast_val));
    mulexp->op2 = op::MOD;
    mulexp->exp2 = unique_ptr<BaseAST>((yyvsp[0].ast_val));
    //mulexp->calc_f=($1->calc_f)&&($3->calc_f);
    (yyval.ast_val) = mulexp;
  }
#line 2465 "src/front/sysy.tab.cpp"
    break;

  case 90:
#line 744 "src/front/sysy.y"
            {
    (yyval.ast_val) = (yyvsp[0].ast_val);
  }
#line 2473 "src/front/sysy.tab.cpp"
    break;

  case 91:
#line 752 "src/front/sysy.y"
             {
    (yyval.ast_val) = (yyvsp[0].ast_val);
  }
#line 2481 "src/front/sysy.tab.cpp"
    break;

  case 92:
#line 755 "src/front/sysy.y"
                    {
    auto ast = new UnaryExpAST();
    ast->op1 = (yyvsp[-1].op_val);
    ast->exp1 = unique_ptr<BaseAST>((yyvsp[0].ast_val));
    //ast->calc_f=ast->exp1->calc_f;
    (yyval.ast_val) = ast;
  }
#line 2493 "src/front/sysy.tab.cpp"
    break;

  case 93:
#line 767 "src/front/sysy.y"
      {
    (yyval.op_val) = op::SUB;
  }
#line 2501 "src/front/sysy.tab.cpp"
    break;

  case 94:
#line 770 "src/front/sysy.y"
      {
    (yyval.op_val) = op::ADD;
  }
#line 2509 "src/front/sysy.tab.cpp"
    break;

  case 95:
#line 773 "src/front/sysy.y"
       {
    (yyval.op_val) = op::NOT;
  }
#line 2517 "src/front/sysy.tab.cpp"
    break;

  case 96:
#line 779 "src/front/sysy.y"
              {
    (yyval.ast_val) = (yyvsp[-1].ast_val);
  }
#line 2525 "src/front/sysy.tab.cpp"
    break;

  case 97:
#line 782 "src/front/sysy.y"
          {
    (yyval.ast_val) = (yyvsp[0].ast_val);
  }
#line 2533 "src/front/sysy.tab.cpp"
    break;

  case 98:
#line 785 "src/front/sysy.y"
       {
    (yyval.ast_val) = (yyvsp[0].ast_val);
  }
#line 2541 "src/front/sysy.tab.cpp"
    break;

  case 99:
#line 788 "src/front/sysy.y"
                             {  ///函数调用
    auto ast = new FuncCallAST();
    ast->ident = *((yyvsp[-3].str_val));
    if((*((yyvsp[-3].str_val))=="starttime"||*((yyvsp[-3].str_val))=="stoptime")&&((yyvsp[-1].ast_val)==nullptr) ){
      ast->ident = (*((yyvsp[-3].str_val))=="starttime") ? "_sysy_starttime" : "_sysy_stoptime";
      //要给其传入行号的参数
      // lineno num
      auto num_ast    = new NumberAST();
      num_ast->value =  int(yylineno);
      num_ast->fvalue = float(yylineno);
      num_ast->calc_f = 1;
      num_ast->val_type =BINT;
      //exp ast
      auto exp_ast    = new ExpAST();
      exp_ast->exp = unique_ptr<BaseAST>(num_ast);
      auto params_ast = new FuncRParamsAST();
      params_ast->paramlist.push_back(unique_ptr<BaseAST>(exp_ast));
      ast->params = unique_ptr<BaseAST>(params_ast);
    }else  ast->params = unique_ptr<BaseAST>((yyvsp[-1].ast_val));

    (yyval.ast_val) = ast;
  }
#line 2568 "src/front/sysy.tab.cpp"
    break;

  case 100:
#line 814 "src/front/sysy.y"
             {
    (yyval.ast_val) = (yyvsp[0].ast_val);
  }
#line 2576 "src/front/sysy.tab.cpp"
    break;

  case 101:
#line 816 "src/front/sysy.y"
               {
    (yyval.ast_val) = nullptr;
  }
#line 2584 "src/front/sysy.tab.cpp"
    break;

  case 102:
#line 823 "src/front/sysy.y"
                     {
    FuncRParamsAST* ast = nullptr;
    ast = dynamic_cast<FuncRParamsAST*>((yyvsp[-2].ast_val));
    if(ast==nullptr){
      std::cerr << "Exception: " << "dynamic_cast failed"<< std::endl;
    }
    ast->paramlist.push_back(unique_ptr<BaseAST>((yyvsp[0].ast_val)));
    (yyval.ast_val) = ast;
  }
#line 2598 "src/front/sysy.tab.cpp"
    break;

  case 103:
#line 832 "src/front/sysy.y"
     {
    auto ast = new FuncRParamsAST();
    ast->paramlist.push_back(unique_ptr<BaseAST>((yyvsp[0].ast_val)));
    (yyval.ast_val) = ast;
  }
#line 2608 "src/front/sysy.tab.cpp"
    break;

  case 104:
#line 841 "src/front/sysy.y"
              {
    auto ast = new NumberAST();
    ast->value = (yyvsp[0].int_val);
    ast->fvalue=float((yyvsp[0].int_val));
    //std::cout<<ast->value<<endl;
    ast->calc_f=1;
    ast->val_type=BINT;
    (yyval.ast_val) = ast;
  }
#line 2622 "src/front/sysy.tab.cpp"
    break;

  case 105:
#line 850 "src/front/sysy.y"
              {
    auto ast = new NumberAST();
    ast->fvalue = (yyvsp[0].float_val);
    ast->value=int((yyvsp[0].float_val));
    //std::cout<<ast->value<<endl;
    ast->calc_f=1;
    ast->val_type=BFLOAT;
    (yyval.ast_val) = ast;
  }
#line 2636 "src/front/sysy.tab.cpp"
    break;

  case 106:
#line 863 "src/front/sysy.y"
        {
    auto ast = new LValAST();
    ast->ident = *((yyvsp[0].str_val));
    (yyval.ast_val) = ast;
  }
#line 2646 "src/front/sysy.tab.cpp"
    break;

  case 107:
#line 868 "src/front/sysy.y"
         {
    auto ast=new LValAST();
    ast->ident="";
    ast->arraydef = unique_ptr<BaseAST>((yyvsp[0].ast_val));
    (yyval.ast_val)=ast;
  }
#line 2657 "src/front/sysy.tab.cpp"
    break;

  case 108:
#line 878 "src/front/sysy.y"
                     {
    LArrayAST* ast = nullptr;
    ast = dynamic_cast<LArrayAST*>((yyvsp[-3].ast_val));
    if(ast==nullptr){
      std::cerr << "Exception: " << "dynamic_cast failed"<< std::endl;
    }
    ast->dimon_list.push_back(unique_ptr<BaseAST>((yyvsp[-1].ast_val)));
    (yyval.ast_val) = ast;
  }
#line 2671 "src/front/sysy.tab.cpp"
    break;

  case 109:
#line 887 "src/front/sysy.y"
        {
    (yyval.ast_val)=(yyvsp[0].ast_val);
  }
#line 2679 "src/front/sysy.tab.cpp"
    break;

  case 110:
#line 894 "src/front/sysy.y"
                   {
    auto ast = new LArrayAST();
    ast->ident = *((yyvsp[-3].str_val));
    ast->dimon_list.push_back(unique_ptr<BaseAST>((yyvsp[-1].ast_val)));
    (yyval.ast_val) = ast;
  }
#line 2690 "src/front/sysy.tab.cpp"
    break;


#line 2694 "src/front/sysy.tab.cpp"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (ast, YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = YY_CAST (char *, YYSTACK_ALLOC (YY_CAST (YYSIZE_T, yymsg_alloc)));
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (ast, yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval, ast);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYTERROR;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  yystos[yystate], yyvsp, ast);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;


#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (ast, YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif


/*-----------------------------------------------------.
| yyreturn -- parsing is finished, return the result.  |
`-----------------------------------------------------*/
yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, ast);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[+*yyssp], yyvsp, ast);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  return yyresult;
}
#line 905 "src/front/sysy.y"


void yyerror(std::unique_ptr<BaseAST> &ast,std::string s) {
  
  extern char *yytext; //define and maintained in lex
  int len = strlen(yytext);
  int i;
  char buf[512]={0};
 

  if(len<512)strcpy(buf, yytext);
  else {
    strncpy(buf, yytext, 511);
    buf[511] = '\0'; // Ensure null termination
  }
  
  if(len==1)fprintf(stderr, "Error: %s at symbol '%c'(symbol len:%d) on line %d\n", s.c_str(), buf[0], len  ,yylineno);
  else fprintf(stderr,      "Error: %s at symbol '%s'(symbol len:%d) on line %d\n", s.c_str(), buf, len  ,yylineno);
  
}
