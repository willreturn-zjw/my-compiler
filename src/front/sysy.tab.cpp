/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
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
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

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

#line 90 "src/front/sysy.tab.cpp"

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

#include "sysy.tab.hpp"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_INT = 3,                        /* INT  */
  YYSYMBOL_VOID = 4,                       /* VOID  */
  YYSYMBOL_FLOAT = 5,                      /* FLOAT  */
  YYSYMBOL_RETURN = 6,                     /* RETURN  */
  YYSYMBOL_CONST = 7,                      /* CONST  */
  YYSYMBOL_IF = 8,                         /* IF  */
  YYSYMBOL_ELSE = 9,                       /* ELSE  */
  YYSYMBOL_WHILE = 10,                     /* WHILE  */
  YYSYMBOL_BREAK = 11,                     /* BREAK  */
  YYSYMBOL_CONTINUE = 12,                  /* CONTINUE  */
  YYSYMBOL_IDENT = 13,                     /* IDENT  */
  YYSYMBOL_INT_CONST = 14,                 /* INT_CONST  */
  YYSYMBOL_FLOAT_CONST = 15,               /* FLOAT_CONST  */
  YYSYMBOL_16_ = 16,                       /* ';'  */
  YYSYMBOL_17_ = 17,                       /* '('  */
  YYSYMBOL_18_ = 18,                       /* ')'  */
  YYSYMBOL_19_ = 19,                       /* ','  */
  YYSYMBOL_20_ = 20,                       /* '['  */
  YYSYMBOL_21_ = 21,                       /* ']'  */
  YYSYMBOL_22_ = 22,                       /* '{'  */
  YYSYMBOL_23_ = 23,                       /* '}'  */
  YYSYMBOL_24_ = 24,                       /* '='  */
  YYSYMBOL_25_ = 25,                       /* '|'  */
  YYSYMBOL_26_ = 26,                       /* '&'  */
  YYSYMBOL_27_ = 27,                       /* '!'  */
  YYSYMBOL_28_ = 28,                       /* '>'  */
  YYSYMBOL_29_ = 29,                       /* '<'  */
  YYSYMBOL_30_ = 30,                       /* '+'  */
  YYSYMBOL_31_ = 31,                       /* '-'  */
  YYSYMBOL_32_ = 32,                       /* '*'  */
  YYSYMBOL_33_ = 33,                       /* '/'  */
  YYSYMBOL_34_ = 34,                       /* '%'  */
  YYSYMBOL_YYACCEPT = 35,                  /* $accept  */
  YYSYMBOL_CompUnit = 36,                  /* CompUnit  */
  YYSYMBOL_DefUnitList = 37,               /* DefUnitList  */
  YYSYMBOL_DefUnits = 38,                  /* DefUnits  */
  YYSYMBOL_DefUnit = 39,                   /* DefUnit  */
  YYSYMBOL_FuncDef = 40,                   /* FuncDef  */
  YYSYMBOL_FuncFParamList = 41,            /* FuncFParamList  */
  YYSYMBOL_FuncFParams = 42,               /* FuncFParams  */
  YYSYMBOL_FuncFParam = 43,                /* FuncFParam  */
  YYSYMBOL_FuncArrays = 44,                /* FuncArrays  */
  YYSYMBOL_FuncArray = 45,                 /* FuncArray  */
  YYSYMBOL_BasicType = 46,                 /* BasicType  */
  YYSYMBOL_Block = 47,                     /* Block  */
  YYSYMBOL_BlockItems = 48,                /* BlockItems  */
  YYSYMBOL_BlockItem = 49,                 /* BlockItem  */
  YYSYMBOL_Stmt = 50,                      /* Stmt  */
  YYSYMBOL_Else = 51,                      /* Else  */
  YYSYMBOL_OptionExp = 52,                 /* OptionExp  */
  YYSYMBOL_Declarationlist = 53,           /* Declarationlist  */
  YYSYMBOL_ConstDeclList = 54,             /* ConstDeclList  */
  YYSYMBOL_VarDeclList = 55,               /* VarDeclList  */
  YYSYMBOL_ConstDefs = 56,                 /* ConstDefs  */
  YYSYMBOL_VarDefs = 57,                   /* VarDefs  */
  YYSYMBOL_ConstDef = 58,                  /* ConstDef  */
  YYSYMBOL_ConstInitVal = 59,              /* ConstInitVal  */
  YYSYMBOL_ConstVals = 60,                 /* ConstVals  */
  YYSYMBOL_VarDef = 61,                    /* VarDef  */
  YYSYMBOL_ArrayDefs = 62,                 /* ArrayDefs  */
  YYSYMBOL_ArrayDef = 63,                  /* ArrayDef  */
  YYSYMBOL_Assignments = 64,               /* Assignments  */
  YYSYMBOL_Assignment = 65,                /* Assignment  */
  YYSYMBOL_InitVal = 66,                   /* InitVal  */
  YYSYMBOL_InitVals = 67,                  /* InitVals  */
  YYSYMBOL_ConstExp = 68,                  /* ConstExp  */
  YYSYMBOL_Exp = 69,                       /* Exp  */
  YYSYMBOL_LOrExp = 70,                    /* LOrExp  */
  YYSYMBOL_LAndExp = 71,                   /* LAndExp  */
  YYSYMBOL_EqExp = 72,                     /* EqExp  */
  YYSYMBOL_RelExp = 73,                    /* RelExp  */
  YYSYMBOL_AddExp = 74,                    /* AddExp  */
  YYSYMBOL_MulExp = 75,                    /* MulExp  */
  YYSYMBOL_UnaryExp = 76,                  /* UnaryExp  */
  YYSYMBOL_UnaryOp = 77,                   /* UnaryOp  */
  YYSYMBOL_PrimaryExp = 78,                /* PrimaryExp  */
  YYSYMBOL_FuncRParamList = 79,            /* FuncRParamList  */
  YYSYMBOL_FuncRParams = 80,               /* FuncRParams  */
  YYSYMBOL_Number = 81,                    /* Number  */
  YYSYMBOL_LVal = 82,                      /* LVal  */
  YYSYMBOL_LArrays = 83,                   /* LArrays  */
  YYSYMBOL_LArray = 84                     /* LArray  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




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

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
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
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
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

#if !defined yyoverflow

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
#endif /* !defined yyoverflow */

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

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   270


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

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

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if YYDEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "INT", "VOID", "FLOAT",
  "RETURN", "CONST", "IF", "ELSE", "WHILE", "BREAK", "CONTINUE", "IDENT",
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

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

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
static const yytype_uint8 yydefgoto[] =
{
       0,     5,     6,     7,     8,     9,    36,    37,    38,    77,
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

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
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

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
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

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
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


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


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

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF


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




# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value, ast); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, std::unique_ptr<BaseAST> &ast)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  YY_USE (ast);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, std::unique_ptr<BaseAST> &ast)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep, ast);
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
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule, std::unique_ptr<BaseAST> &ast)
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
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)], ast);
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
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
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






/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep, std::unique_ptr<BaseAST> &ast)
{
  YY_USE (yyvaluep);
  YY_USE (ast);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
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
    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

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
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
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
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
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

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
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
  case 2: /* CompUnit: DefUnitList  */
#line 65 "src/front/sysy.y"
              {
   auto comp_unit = make_unique<CompUnitAST>();
   comp_unit->start = unique_ptr<BaseAST>((yyvsp[0].ast_val));
   ast = std::move(comp_unit);
 }
#line 1314 "src/front/sysy.tab.cpp"
    break;

  case 3: /* DefUnitList: DefUnits  */
#line 72 "src/front/sysy.y"
           {
    (yyval.ast_val) = (yyvsp[0].ast_val);
  }
#line 1322 "src/front/sysy.tab.cpp"
    break;

  case 4: /* DefUnits: DefUnits DefUnit  */
#line 77 "src/front/sysy.y"
                    {
    DefUnitsAST* ast = dynamic_cast<DefUnitsAST*>((yyvsp[-1].ast_val));
    if(ast==nullptr){
      std::cerr << "Exception: " << "dynamic_cast failed"<< std::endl;
    }
    ast->unit_list.push_back(unique_ptr<BaseAST>((yyvsp[0].ast_val)));
    (yyval.ast_val) = ast;
  }
#line 1335 "src/front/sysy.tab.cpp"
    break;

  case 5: /* DefUnits: DefUnit  */
#line 84 "src/front/sysy.y"
            {
    auto ast = new DefUnitsAST();
    ast->unit_list.push_back(unique_ptr<BaseAST>((yyvsp[0].ast_val)));
    (yyval.ast_val) = ast;
  }
#line 1345 "src/front/sysy.tab.cpp"
    break;

  case 6: /* DefUnit: FuncDef  */
#line 92 "src/front/sysy.y"
            {
    auto ast = new DefUnitAST();
    ast->content = unique_ptr<BaseAST>((yyvsp[0].ast_val));
    (yyval.ast_val)=ast;
  }
#line 1355 "src/front/sysy.tab.cpp"
    break;

  case 7: /* DefUnit: Declarationlist ';'  */
#line 97 "src/front/sysy.y"
                     {
    auto ast = new DefUnitAST();
    ast->content = unique_ptr<BaseAST>((yyvsp[-1].ast_val));
    (yyval.ast_val)=ast;
  }
#line 1365 "src/front/sysy.tab.cpp"
    break;

  case 8: /* FuncDef: BasicType IDENT '(' FuncFParamList ')' Block  */
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
#line 1380 "src/front/sysy.tab.cpp"
    break;

  case 9: /* FuncFParamList: FuncFParams  */
#line 121 "src/front/sysy.y"
              {
    (yyval.ast_val) = (yyvsp[0].ast_val);
  }
#line 1388 "src/front/sysy.tab.cpp"
    break;

  case 10: /* FuncFParamList: %empty  */
#line 124 "src/front/sysy.y"
  {
    (yyval.ast_val) = nullptr;
  }
#line 1396 "src/front/sysy.tab.cpp"
    break;

  case 11: /* FuncFParams: FuncFParams ',' FuncFParam  */
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
#line 1410 "src/front/sysy.tab.cpp"
    break;

  case 12: /* FuncFParams: FuncFParam  */
#line 139 "src/front/sysy.y"
             {
    auto ast = new FuncFParamsAST();
    ast->paramlist.push_back(unique_ptr<BaseAST>((yyvsp[0].ast_val)));
    (yyval.ast_val) = ast;
  }
#line 1420 "src/front/sysy.tab.cpp"
    break;

  case 13: /* FuncFParam: BasicType IDENT  */
#line 147 "src/front/sysy.y"
                    {
      auto ast = new FuncFParamAST();
      ast->type = (yyvsp[-1].btype_val);
      var_type=(yyvsp[-1].btype_val);
      ast->ident = *((yyvsp[0].str_val));
      ast->arraydef = nullptr;
      (yyval.ast_val) = ast;
    }
#line 1433 "src/front/sysy.tab.cpp"
    break;

  case 14: /* FuncFParam: BasicType FuncArrays  */
#line 155 "src/front/sysy.y"
                         {
      auto ast = new FuncFParamAST();
      var_type=(yyvsp[-1].btype_val);
      ast->type = (yyvsp[-1].btype_val);
      ast->ident = "";
      ast->arraydef = unique_ptr<BaseAST>((yyvsp[0].ast_val));
      (yyval.ast_val) = ast;
    }
#line 1446 "src/front/sysy.tab.cpp"
    break;

  case 15: /* FuncArrays: FuncArrays '[' ConstExp ']'  */
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
#line 1460 "src/front/sysy.tab.cpp"
    break;

  case 16: /* FuncArrays: FuncArray  */
#line 175 "src/front/sysy.y"
            {
    (yyval.ast_val) = (yyvsp[0].ast_val);
  }
#line 1468 "src/front/sysy.tab.cpp"
    break;

  case 17: /* FuncArray: IDENT '[' ']'  */
#line 181 "src/front/sysy.y"
               {
    auto ast = new FuncArrayAST();
    ast->ident = *((yyvsp[-2].str_val));
    (yyval.ast_val) = ast;
  }
#line 1478 "src/front/sysy.tab.cpp"
    break;

  case 18: /* BasicType: INT  */
#line 196 "src/front/sysy.y"
               {
  (yyval.btype_val) = Btype::BINT;
}
#line 1486 "src/front/sysy.tab.cpp"
    break;

  case 19: /* BasicType: VOID  */
#line 199 "src/front/sysy.y"
        {
    (yyval.btype_val) = Btype::BVOID;
}
#line 1494 "src/front/sysy.tab.cpp"
    break;

  case 20: /* BasicType: FLOAT  */
#line 202 "src/front/sysy.y"
         {
    (yyval.btype_val) = Btype::BFLOAT;
}
#line 1502 "src/front/sysy.tab.cpp"
    break;

  case 21: /* Block: '{' BlockItems '}'  */
#line 212 "src/front/sysy.y"
                       {
    auto ast=new BlockAST();
    ast->blockitems = unique_ptr<BaseAST>((yyvsp[-1].ast_val));
    (yyval.ast_val)=ast;
  }
#line 1512 "src/front/sysy.tab.cpp"
    break;

  case 22: /* Block: '{' '}'  */
#line 216 "src/front/sysy.y"
           {
    auto ast=new BlockAST();
    ast->blockitems=nullptr;
    (yyval.ast_val)=ast;
  }
#line 1522 "src/front/sysy.tab.cpp"
    break;

  case 23: /* BlockItems: BlockItems BlockItem  */
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
#line 1539 "src/front/sysy.tab.cpp"
    break;

  case 24: /* BlockItems: BlockItem  */
#line 237 "src/front/sysy.y"
              {
      auto Items = new BlockItemsAST();
      Items->itemlist.push_back(unique_ptr<BaseAST>((yyvsp[0].ast_val)));
      (yyval.ast_val) = Items;
    }
#line 1549 "src/front/sysy.tab.cpp"
    break;

  case 25: /* BlockItem: Declarationlist ';'  */
#line 246 "src/front/sysy.y"
                     {
    auto ast = new BlockItemAST();
    ast->content = unique_ptr<BaseAST>((yyvsp[-1].ast_val));
    (yyval.ast_val)=ast;
  }
#line 1559 "src/front/sysy.tab.cpp"
    break;

  case 26: /* BlockItem: Stmt  */
#line 251 "src/front/sysy.y"
       {
    auto ast = new StmtAST();
    ast->sent = unique_ptr<BaseAST>((yyvsp[0].ast_val));
    (yyval.ast_val) = ast;
  }
#line 1569 "src/front/sysy.tab.cpp"
    break;

  case 27: /* Stmt: Assignments ';'  */
#line 260 "src/front/sysy.y"
                  {
    auto ast =new BlockItemAST();
    ast->content = unique_ptr<BaseAST>((yyvsp[-1].ast_val));
    (yyval.ast_val)=ast;
  }
#line 1579 "src/front/sysy.tab.cpp"
    break;

  case 28: /* Stmt: RETURN Exp ';'  */
#line 265 "src/front/sysy.y"
                 {
    auto ast = new BlockItemAST();
    auto retAst=new ReturnAST();
    retAst->retNum=unique_ptr<BaseAST>((yyvsp[-1].ast_val));
    ast->content=unique_ptr<BaseAST>(retAst);
    (yyval.ast_val)=ast;
  }
#line 1591 "src/front/sysy.tab.cpp"
    break;

  case 29: /* Stmt: OptionExp ';'  */
#line 272 "src/front/sysy.y"
                {
    auto ast = new BlockItemAST();
    ast->content = unique_ptr<BaseAST>((yyvsp[-1].ast_val));
    (yyval.ast_val)=ast;
  }
#line 1601 "src/front/sysy.tab.cpp"
    break;

  case 30: /* Stmt: RETURN ';'  */
#line 277 "src/front/sysy.y"
              {
    auto ast = new BlockItemAST();
    auto retAst=new ReturnAST();
    retAst->retNum=nullptr;
    ast->content=unique_ptr<BaseAST>(retAst);
    (yyval.ast_val)=ast;
  }
#line 1613 "src/front/sysy.tab.cpp"
    break;

  case 31: /* Stmt: Block  */
#line 284 "src/front/sysy.y"
       {
    auto ast = new BlockItemAST();
    ast->content = unique_ptr<BaseAST>((yyvsp[0].ast_val));
    (yyval.ast_val)=ast;
  }
#line 1623 "src/front/sysy.tab.cpp"
    break;

  case 32: /* Stmt: IF '(' Exp ')' Stmt Else  */
#line 289 "src/front/sysy.y"
                          {
    auto ast=new IfElseAST();
    ast->exp=unique_ptr<BaseAST>((yyvsp[-3].ast_val));
    ast->then_part=unique_ptr<BaseAST>((yyvsp[-1].ast_val));
    ast->else_part=unique_ptr<BaseAST>((yyvsp[0].ast_val));
    (yyval.ast_val)=ast; 
  }
#line 1635 "src/front/sysy.tab.cpp"
    break;

  case 33: /* Stmt: WHILE '(' Exp ')' Stmt  */
#line 296 "src/front/sysy.y"
                        {
    auto ast=new WhileAST();
    ast->exp=unique_ptr<BaseAST>((yyvsp[-2].ast_val));
    ast->body=unique_ptr<BaseAST>((yyvsp[0].ast_val));
    (yyval.ast_val)=ast;
  }
#line 1646 "src/front/sysy.tab.cpp"
    break;

  case 34: /* Stmt: BREAK ';'  */
#line 302 "src/front/sysy.y"
           {
    auto ast=new BreakAST();
    (yyval.ast_val)=ast;
  }
#line 1655 "src/front/sysy.tab.cpp"
    break;

  case 35: /* Stmt: CONTINUE ';'  */
#line 306 "src/front/sysy.y"
              {
    auto ast=new ContinueAST();
    (yyval.ast_val)=ast;
  }
#line 1664 "src/front/sysy.tab.cpp"
    break;

  case 36: /* Else: ELSE Stmt  */
#line 315 "src/front/sysy.y"
           {
    (yyval.ast_val)=(yyvsp[0].ast_val);
  }
#line 1672 "src/front/sysy.tab.cpp"
    break;

  case 37: /* Else: %empty  */
#line 319 "src/front/sysy.y"
  {
    (yyval.ast_val)=nullptr;
  }
#line 1680 "src/front/sysy.tab.cpp"
    break;

  case 38: /* OptionExp: Exp  */
#line 325 "src/front/sysy.y"
       {
      auto ast=new OptionExpAST();
      ast->exp=unique_ptr<BaseAST>((yyvsp[0].ast_val));
      (yyval.ast_val)=ast;
    }
#line 1690 "src/front/sysy.tab.cpp"
    break;

  case 39: /* OptionExp: %empty  */
#line 331 "src/front/sysy.y"
    {
      auto ast=new OptionExpAST();
      ast->exp=nullptr;
      (yyval.ast_val) = ast;
    }
#line 1700 "src/front/sysy.tab.cpp"
    break;

  case 40: /* Declarationlist: ConstDeclList  */
#line 340 "src/front/sysy.y"
                {
    (yyval.ast_val) = (yyvsp[0].ast_val);
  }
#line 1708 "src/front/sysy.tab.cpp"
    break;

  case 41: /* Declarationlist: VarDeclList  */
#line 343 "src/front/sysy.y"
              {
    (yyval.ast_val) = (yyvsp[0].ast_val);
  }
#line 1716 "src/front/sysy.tab.cpp"
    break;

  case 42: /* ConstDeclList: CONST BasicType ConstDefs  */
#line 349 "src/front/sysy.y"
                            {
    auto constdecllist = new ConstDeclListAST();
    constdecllist->type = (yyvsp[-1].btype_val);
    var_type=(yyvsp[-1].btype_val);
    constdecllist->constdefs = unique_ptr<BaseAST>((yyvsp[0].ast_val));
    (yyval.ast_val) = constdecllist;
  }
#line 1728 "src/front/sysy.tab.cpp"
    break;

  case 43: /* VarDeclList: BasicType VarDefs  */
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
#line 1742 "src/front/sysy.tab.cpp"
    break;

  case 44: /* ConstDefs: ConstDef  */
#line 370 "src/front/sysy.y"
           {
    auto Defs = new ConstDefsAST();
    Defs->constdefs.push_back(unique_ptr<BaseAST>((yyvsp[0].ast_val)));
    (yyval.ast_val) = Defs;
  }
#line 1752 "src/front/sysy.tab.cpp"
    break;

  case 45: /* ConstDefs: ConstDefs ',' ConstDef  */
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
#line 1767 "src/front/sysy.tab.cpp"
    break;

  case 46: /* VarDefs: VarDef  */
#line 389 "src/front/sysy.y"
         {
    auto Defs = new VarDefsAST();
    Defs->vardefs.push_back(unique_ptr<BaseAST>((yyvsp[0].ast_val)));
    //std::cout <<"VarDefs"<<Defs->type << std::endl; 
    (yyval.ast_val) = Defs;
  }
#line 1778 "src/front/sysy.tab.cpp"
    break;

  case 47: /* VarDefs: VarDefs ',' VarDef  */
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
#line 1793 "src/front/sysy.tab.cpp"
    break;

  case 48: /* ConstDef: IDENT '=' ConstInitVal  */
#line 408 "src/front/sysy.y"
                         {
    auto ast = new ConstDefAST();
    ast->ident = *((yyvsp[-2].str_val));
    ast->arraydef = nullptr;
    ast->initval = unique_ptr<BaseAST>((yyvsp[0].ast_val));
    (yyval.ast_val) = ast;
  }
#line 1805 "src/front/sysy.tab.cpp"
    break;

  case 49: /* ConstDef: ArrayDefs '=' ConstInitVal  */
#line 416 "src/front/sysy.y"
                             {
    auto ast = new ConstDefAST();
    ast->ident = "";
    ast->arraydef = unique_ptr<BaseAST>((yyvsp[-2].ast_val));
    ast->initval = unique_ptr<BaseAST>((yyvsp[0].ast_val));
    (yyval.ast_val) = ast;
  }
#line 1817 "src/front/sysy.tab.cpp"
    break;

  case 50: /* ConstInitVal: ConstExp  */
#line 426 "src/front/sysy.y"
           {
    auto initval = new ConstExpAST();
    initval->exp = unique_ptr<BaseAST>((yyvsp[0].ast_val));
    (yyval.ast_val) = initval;
  }
#line 1827 "src/front/sysy.tab.cpp"
    break;

  case 51: /* ConstInitVal: '{' ConstVals '}'  */
#line 431 "src/front/sysy.y"
                    {
    (yyval.ast_val)=(yyvsp[-1].ast_val);
  }
#line 1835 "src/front/sysy.tab.cpp"
    break;

  case 52: /* ConstInitVal: '{' '}'  */
#line 434 "src/front/sysy.y"
         {
    auto initvals = new ConstInitValsAST();
    initvals->constexp = nullptr;
    (yyval.ast_val) = initvals;
  }
#line 1845 "src/front/sysy.tab.cpp"
    break;

  case 53: /* ConstVals: ConstInitVal  */
#line 442 "src/front/sysy.y"
               {
    auto ast = new ConstInitValsAST();
    ast->array_val_list.push_back(unique_ptr<BaseAST>((yyvsp[0].ast_val)));
    (yyval.ast_val) = ast;
  }
#line 1855 "src/front/sysy.tab.cpp"
    break;

  case 54: /* ConstVals: ConstVals ',' ConstInitVal  */
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
#line 1869 "src/front/sysy.tab.cpp"
    break;

  case 55: /* VarDef: IDENT '=' InitVal  */
#line 462 "src/front/sysy.y"
                    {
    auto ast = new VarDefAST();
    ast->ident = *((yyvsp[-2].str_val));
    ast->arraydef = nullptr;
    ast->initval = unique_ptr<BaseAST>((yyvsp[0].ast_val));
    (yyval.ast_val) = ast;
  }
#line 1881 "src/front/sysy.tab.cpp"
    break;

  case 56: /* VarDef: IDENT  */
#line 469 "src/front/sysy.y"
         {
    auto ast = new VarDefAST();
    ast->ident = *((yyvsp[0].str_val));
    ast->arraydef = nullptr;
    ast->initval = nullptr;
    (yyval.ast_val) = ast;
  }
#line 1893 "src/front/sysy.tab.cpp"
    break;

  case 57: /* VarDef: ArrayDefs  */
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
#line 1908 "src/front/sysy.tab.cpp"
    break;

  case 58: /* VarDef: ArrayDefs '=' InitVal  */
#line 487 "src/front/sysy.y"
                         {
    auto ast = new VarDefAST();
    ast->ident = "";
    ast->arraydef = unique_ptr<BaseAST>((yyvsp[-2].ast_val));
    ast->initval = unique_ptr<BaseAST>((yyvsp[0].ast_val));
    (yyval.ast_val) = ast;
  }
#line 1920 "src/front/sysy.tab.cpp"
    break;

  case 59: /* ArrayDefs: ArrayDefs '[' ConstExp ']'  */
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
#line 1934 "src/front/sysy.tab.cpp"
    break;

  case 60: /* ArrayDefs: ArrayDef  */
#line 507 "src/front/sysy.y"
           {
    (yyval.ast_val) = (yyvsp[0].ast_val);
  }
#line 1942 "src/front/sysy.tab.cpp"
    break;

  case 61: /* ArrayDef: IDENT '[' ConstExp ']'  */
#line 513 "src/front/sysy.y"
                         {
    auto ast = new ArrayDefsAST();
    ast->ident = *((yyvsp[-3].str_val));
    ast->dimon_list.push_back(unique_ptr<BaseAST>((yyvsp[-1].ast_val)));
    (yyval.ast_val) = ast;
  }
#line 1953 "src/front/sysy.tab.cpp"
    break;

  case 62: /* Assignments: Assignment  */
#line 522 "src/front/sysy.y"
               {
    (yyval.ast_val)=(yyvsp[0].ast_val);
  }
#line 1961 "src/front/sysy.tab.cpp"
    break;

  case 63: /* Assignment: IDENT '=' Exp  */
#line 528 "src/front/sysy.y"
                {
    auto ast=new AssignAST();
    ast->ident=*((yyvsp[-2].str_val));
    ast->arraydef=nullptr;
    ast->exp=unique_ptr<BaseAST>((yyvsp[0].ast_val));
    //set_sym_val(sym_head,sym_tail,*($1),$3);
    (yyval.ast_val)=ast;
  }
#line 1974 "src/front/sysy.tab.cpp"
    break;

  case 64: /* Assignment: LArrays '=' Exp  */
#line 536 "src/front/sysy.y"
                 {
    auto ast=new AssignAST();
    ast->ident="";
    ast->arraydef=unique_ptr<BaseAST>((yyvsp[-2].ast_val));
    ast->exp=unique_ptr<BaseAST>((yyvsp[0].ast_val));
    (yyval.ast_val)=ast;
  }
#line 1986 "src/front/sysy.tab.cpp"
    break;

  case 65: /* InitVal: Exp  */
#line 547 "src/front/sysy.y"
      {
    auto initval = new ExpAST();
    initval->exp = unique_ptr<BaseAST>((yyvsp[0].ast_val));
    (yyval.ast_val) = initval;
  }
#line 1996 "src/front/sysy.tab.cpp"
    break;

  case 66: /* InitVal: '{' InitVals '}'  */
#line 553 "src/front/sysy.y"
                  {
    InitValsAST* ast=dynamic_cast<InitValsAST*>((yyvsp[-1].ast_val));
    if(ast==nullptr){
      throw std::runtime_error("error: ArrayDefsAST cast failed");
    }
    (yyval.ast_val) = ast;
  }
#line 2008 "src/front/sysy.tab.cpp"
    break;

  case 67: /* InitVal: '{' '}'  */
#line 561 "src/front/sysy.y"
         {
    auto initvals = new InitValsAST();
    initvals->exp = nullptr;
    (yyval.ast_val) = initvals;
  }
#line 2018 "src/front/sysy.tab.cpp"
    break;

  case 68: /* InitVals: InitVal  */
#line 569 "src/front/sysy.y"
          {
    auto ast = new InitValsAST();
    ast->array_val_list.push_back(unique_ptr<BaseAST>((yyvsp[0].ast_val)));
    (yyval.ast_val) = ast;
  }
#line 2028 "src/front/sysy.tab.cpp"
    break;

  case 69: /* InitVals: InitVals ',' InitVal  */
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
#line 2042 "src/front/sysy.tab.cpp"
    break;

  case 70: /* ConstExp: Exp  */
#line 585 "src/front/sysy.y"
     {
    auto ast=new ConstExpAST();
    ast->exp=unique_ptr<BaseAST>((yyvsp[0].ast_val));
    (yyval.ast_val)=ast;
  }
#line 2052 "src/front/sysy.tab.cpp"
    break;

  case 71: /* Exp: LOrExp  */
#line 595 "src/front/sysy.y"
           {
    auto ast=new ExpAST();
    ast->exp=unique_ptr<BaseAST>((yyvsp[0].ast_val));
    (yyval.ast_val)=ast;
  }
#line 2062 "src/front/sysy.tab.cpp"
    break;

  case 72: /* LOrExp: LOrExp '|' '|' LAndExp  */
#line 604 "src/front/sysy.y"
                        {
    auto lorexp = new BinaryExpAST();
    lorexp->exp1 = unique_ptr<BaseAST>((yyvsp[-3].ast_val));
    lorexp->exp2 = unique_ptr<BaseAST>((yyvsp[0].ast_val));
    lorexp->op2=op::OR;
    //lorexp->calc_f=($1->calc_f)&&($4->calc_f);
    (yyval.ast_val) = lorexp;
  }
#line 2075 "src/front/sysy.tab.cpp"
    break;

  case 73: /* LOrExp: LAndExp  */
#line 612 "src/front/sysy.y"
            {
    (yyval.ast_val)=(yyvsp[0].ast_val);
  }
#line 2083 "src/front/sysy.tab.cpp"
    break;

  case 74: /* LAndExp: LAndExp '&' '&' EqExp  */
#line 619 "src/front/sysy.y"
                       {
    auto landexp = new BinaryExpAST();
    landexp->exp1 = unique_ptr<BaseAST>((yyvsp[-3].ast_val));
    landexp->exp2 = unique_ptr<BaseAST>((yyvsp[0].ast_val));
    landexp->op2 = op::AND;
    //landexp->calc_f=($1->calc_f)&&($4->calc_f);
    (yyval.ast_val) = landexp;
  }
#line 2096 "src/front/sysy.tab.cpp"
    break;

  case 75: /* LAndExp: EqExp  */
#line 627 "src/front/sysy.y"
          {
    (yyval.ast_val)=(yyvsp[0].ast_val);
  }
#line 2104 "src/front/sysy.tab.cpp"
    break;

  case 76: /* EqExp: EqExp '=' '=' RelExp  */
#line 634 "src/front/sysy.y"
                      {
    auto eqexp = new BinaryExpAST();
    eqexp->exp1 = unique_ptr<BaseAST>((yyvsp[-3].ast_val));
    eqexp->exp2 = unique_ptr<BaseAST>((yyvsp[0].ast_val));
    eqexp->op2 = op::EQ;
    //eqexp->calc_f=($1->calc_f)&&($4->calc_f);
    (yyval.ast_val) = eqexp;
  }
#line 2117 "src/front/sysy.tab.cpp"
    break;

  case 77: /* EqExp: EqExp '!' '=' RelExp  */
#line 642 "src/front/sysy.y"
                         {
    auto eqexp = new BinaryExpAST();
    eqexp->exp1 = unique_ptr<BaseAST>((yyvsp[-3].ast_val));
    eqexp->exp2 = unique_ptr<BaseAST>((yyvsp[0].ast_val));
    eqexp->op2 = op::NE;
    //eqexp->calc_f=($1->calc_f)&&($4->calc_f);
    (yyval.ast_val) = eqexp;
  }
#line 2130 "src/front/sysy.tab.cpp"
    break;

  case 78: /* EqExp: RelExp  */
#line 650 "src/front/sysy.y"
           {
    (yyval.ast_val)=(yyvsp[0].ast_val);
  }
#line 2138 "src/front/sysy.tab.cpp"
    break;

  case 79: /* RelExp: RelExp '>' AddExp  */
#line 658 "src/front/sysy.y"
                    {
    auto relexp = new BinaryExpAST();
    relexp->exp1 = unique_ptr<BaseAST>((yyvsp[-2].ast_val));
    relexp->exp2 = unique_ptr<BaseAST>((yyvsp[0].ast_val));
    relexp->op2 = op::GT;
    //relexp->calc_f=($1->calc_f)&&($3->calc_f);
    (yyval.ast_val) = relexp;
  }
#line 2151 "src/front/sysy.tab.cpp"
    break;

  case 80: /* RelExp: RelExp '<' AddExp  */
#line 666 "src/front/sysy.y"
                      {
    auto relexp = new BinaryExpAST();
    relexp->exp1 = unique_ptr<BaseAST>((yyvsp[-2].ast_val));
    relexp->exp2 = unique_ptr<BaseAST>((yyvsp[0].ast_val));
    relexp->op2 = op::LT;
    //relexp->calc_f=($1->calc_f)&&($3->calc_f);
    (yyval.ast_val) = relexp;
  }
#line 2164 "src/front/sysy.tab.cpp"
    break;

  case 81: /* RelExp: RelExp '>' '=' AddExp  */
#line 674 "src/front/sysy.y"
                         {
    auto relexp = new BinaryExpAST();
    relexp->exp1 = unique_ptr<BaseAST>((yyvsp[-3].ast_val));
    relexp->exp2 = unique_ptr<BaseAST>((yyvsp[0].ast_val));
    relexp->op2 = op::GE;
    //relexp->calc_f=($1->calc_f)&&($4->calc_f);
    (yyval.ast_val) = relexp;
  }
#line 2177 "src/front/sysy.tab.cpp"
    break;

  case 82: /* RelExp: RelExp '<' '=' AddExp  */
#line 682 "src/front/sysy.y"
                         {
    auto relexp = new BinaryExpAST();
    relexp->exp1 = unique_ptr<BaseAST>((yyvsp[-3].ast_val));
    relexp->exp2 = unique_ptr<BaseAST>((yyvsp[0].ast_val));
    relexp->op2 = op::LE;
    //relexp->calc_f=($1->calc_f)&&($4->calc_f);
    (yyval.ast_val) = relexp;
  }
#line 2190 "src/front/sysy.tab.cpp"
    break;

  case 83: /* RelExp: AddExp  */
#line 690 "src/front/sysy.y"
           {
    (yyval.ast_val)=(yyvsp[0].ast_val);
  }
#line 2198 "src/front/sysy.tab.cpp"
    break;

  case 84: /* AddExp: AddExp '+' MulExp  */
#line 697 "src/front/sysy.y"
                    {
    auto addexp = new BinaryExpAST();
    addexp->exp1 = unique_ptr<BaseAST>((yyvsp[-2].ast_val));
    addexp->exp2 = unique_ptr<BaseAST>((yyvsp[0].ast_val));
    addexp->op2 = op::ADD;
    //addexp->calc_f=($1->calc_f)&&($3->calc_f);
    (yyval.ast_val) = addexp;
  }
#line 2211 "src/front/sysy.tab.cpp"
    break;

  case 85: /* AddExp: AddExp '-' MulExp  */
#line 705 "src/front/sysy.y"
                      {
    auto addexp = new BinaryExpAST();
    addexp->exp1 = unique_ptr<BaseAST>((yyvsp[-2].ast_val));
    addexp->exp2 = unique_ptr<BaseAST>((yyvsp[0].ast_val));
    addexp->op2 = op::SUB;
    //addexp->calc_f=($1->calc_f)&&($3->calc_f);
    (yyval.ast_val) = addexp;
  }
#line 2224 "src/front/sysy.tab.cpp"
    break;

  case 86: /* AddExp: MulExp  */
#line 713 "src/front/sysy.y"
           {
    (yyval.ast_val)=(yyvsp[0].ast_val);
  }
#line 2232 "src/front/sysy.tab.cpp"
    break;

  case 87: /* MulExp: MulExp '*' UnaryExp  */
#line 720 "src/front/sysy.y"
                      {
    auto mulexp = new BinaryExpAST();
    mulexp->exp1 = unique_ptr<BaseAST>((yyvsp[-2].ast_val));
    mulexp->op2 = op::MUL;
    mulexp->exp2 = unique_ptr<BaseAST>((yyvsp[0].ast_val));
    //mulexp->calc_f=($1->calc_f)&&($3->calc_f);
    (yyval.ast_val) = mulexp;
  }
#line 2245 "src/front/sysy.tab.cpp"
    break;

  case 88: /* MulExp: MulExp '/' UnaryExp  */
#line 728 "src/front/sysy.y"
                        {
    auto mulexp = new BinaryExpAST();
    mulexp->exp1 = unique_ptr<BaseAST>((yyvsp[-2].ast_val));
    mulexp->op2 = op::DIV;
    mulexp->exp2 = unique_ptr<BaseAST>((yyvsp[0].ast_val));
    //mulexp->calc_f=($1->calc_f)&&($3->calc_f);
    (yyval.ast_val) = mulexp;
  }
#line 2258 "src/front/sysy.tab.cpp"
    break;

  case 89: /* MulExp: MulExp '%' UnaryExp  */
#line 736 "src/front/sysy.y"
                        {
    auto mulexp = new BinaryExpAST();
    mulexp->exp1 = unique_ptr<BaseAST>((yyvsp[-2].ast_val));
    mulexp->op2 = op::MOD;
    mulexp->exp2 = unique_ptr<BaseAST>((yyvsp[0].ast_val));
    //mulexp->calc_f=($1->calc_f)&&($3->calc_f);
    (yyval.ast_val) = mulexp;
  }
#line 2271 "src/front/sysy.tab.cpp"
    break;

  case 90: /* MulExp: UnaryExp  */
#line 744 "src/front/sysy.y"
            {
    (yyval.ast_val) = (yyvsp[0].ast_val);
  }
#line 2279 "src/front/sysy.tab.cpp"
    break;

  case 91: /* UnaryExp: PrimaryExp  */
#line 752 "src/front/sysy.y"
             {
    (yyval.ast_val) = (yyvsp[0].ast_val);
  }
#line 2287 "src/front/sysy.tab.cpp"
    break;

  case 92: /* UnaryExp: UnaryOp UnaryExp  */
#line 755 "src/front/sysy.y"
                    {
    auto ast = new UnaryExpAST();
    ast->op1 = (yyvsp[-1].op_val);
    ast->exp1 = unique_ptr<BaseAST>((yyvsp[0].ast_val));
    //ast->calc_f=ast->exp1->calc_f;
    (yyval.ast_val) = ast;
  }
#line 2299 "src/front/sysy.tab.cpp"
    break;

  case 93: /* UnaryOp: '-'  */
#line 767 "src/front/sysy.y"
      {
    (yyval.op_val) = op::SUB;
  }
#line 2307 "src/front/sysy.tab.cpp"
    break;

  case 94: /* UnaryOp: '+'  */
#line 770 "src/front/sysy.y"
      {
    (yyval.op_val) = op::ADD;
  }
#line 2315 "src/front/sysy.tab.cpp"
    break;

  case 95: /* UnaryOp: '!'  */
#line 773 "src/front/sysy.y"
       {
    (yyval.op_val) = op::NOT;
  }
#line 2323 "src/front/sysy.tab.cpp"
    break;

  case 96: /* PrimaryExp: '(' Exp ')'  */
#line 779 "src/front/sysy.y"
              {
    (yyval.ast_val) = (yyvsp[-1].ast_val);
  }
#line 2331 "src/front/sysy.tab.cpp"
    break;

  case 97: /* PrimaryExp: Number  */
#line 782 "src/front/sysy.y"
          {
    (yyval.ast_val) = (yyvsp[0].ast_val);
  }
#line 2339 "src/front/sysy.tab.cpp"
    break;

  case 98: /* PrimaryExp: LVal  */
#line 785 "src/front/sysy.y"
       {
    (yyval.ast_val) = (yyvsp[0].ast_val);
  }
#line 2347 "src/front/sysy.tab.cpp"
    break;

  case 99: /* PrimaryExp: IDENT '(' FuncRParamList ')'  */
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
#line 2374 "src/front/sysy.tab.cpp"
    break;

  case 100: /* FuncRParamList: FuncRParams  */
#line 814 "src/front/sysy.y"
             {
    (yyval.ast_val) = (yyvsp[0].ast_val);
  }
#line 2382 "src/front/sysy.tab.cpp"
    break;

  case 101: /* FuncRParamList: %empty  */
#line 816 "src/front/sysy.y"
               {
    (yyval.ast_val) = nullptr;
  }
#line 2390 "src/front/sysy.tab.cpp"
    break;

  case 102: /* FuncRParams: FuncRParams ',' Exp  */
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
#line 2404 "src/front/sysy.tab.cpp"
    break;

  case 103: /* FuncRParams: Exp  */
#line 832 "src/front/sysy.y"
     {
    auto ast = new FuncRParamsAST();
    ast->paramlist.push_back(unique_ptr<BaseAST>((yyvsp[0].ast_val)));
    (yyval.ast_val) = ast;
  }
#line 2414 "src/front/sysy.tab.cpp"
    break;

  case 104: /* Number: INT_CONST  */
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
#line 2428 "src/front/sysy.tab.cpp"
    break;

  case 105: /* Number: FLOAT_CONST  */
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
#line 2442 "src/front/sysy.tab.cpp"
    break;

  case 106: /* LVal: IDENT  */
#line 863 "src/front/sysy.y"
        {
    auto ast = new LValAST();
    ast->ident = *((yyvsp[0].str_val));
    (yyval.ast_val) = ast;
  }
#line 2452 "src/front/sysy.tab.cpp"
    break;

  case 107: /* LVal: LArrays  */
#line 868 "src/front/sysy.y"
         {
    auto ast=new LValAST();
    ast->ident="";
    ast->arraydef = unique_ptr<BaseAST>((yyvsp[0].ast_val));
    (yyval.ast_val)=ast;
  }
#line 2463 "src/front/sysy.tab.cpp"
    break;

  case 108: /* LArrays: LArrays '[' Exp ']'  */
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
#line 2477 "src/front/sysy.tab.cpp"
    break;

  case 109: /* LArrays: LArray  */
#line 887 "src/front/sysy.y"
        {
    (yyval.ast_val)=(yyvsp[0].ast_val);
  }
#line 2485 "src/front/sysy.tab.cpp"
    break;

  case 110: /* LArray: IDENT '[' Exp ']'  */
#line 894 "src/front/sysy.y"
                   {
    auto ast = new LArrayAST();
    ast->ident = *((yyvsp[-3].str_val));
    ast->dimon_list.push_back(unique_ptr<BaseAST>((yyvsp[-1].ast_val)));
    (yyval.ast_val) = ast;
  }
#line 2496 "src/front/sysy.tab.cpp"
    break;


#line 2500 "src/front/sysy.tab.cpp"

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
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

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
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      yyerror (ast, YY_("syntax error"));
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
  ++yynerrs;

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

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
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
                  YY_ACCESSING_SYMBOL (yystate), yyvsp, ast);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (ast, YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
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
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp, ast);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
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
