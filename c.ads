with Interfaces.C;
with System;

package C is

   --  ****************************************
   --  Types corresponding to C built-in types:
   --  ****************************************

   subtype char           is Interfaces.C.char;
   subtype signed_char    is Interfaces.C.signed_char;
   subtype unsigned_char  is Interfaces.C.unsigned_char;

   subtype short          is Interfaces.C.short;
   subtype unsigned_short is Interfaces.C.unsigned_short;

   subtype int            is Interfaces.C.int;
   subtype natural_int    is int range 0 .. int'Last;    -- array indices
   subtype unsigned_int   is Interfaces.C.unsigned;

   subtype long           is Interfaces.C.long;
   subtype unsigned_long  is Interfaces.C.unsigned_long;

   subtype float          is Interfaces.C.C_float;
   subtype double         is Interfaces.C.double;

   --  subtype charp       is X.Strings.charp;
   type charp             is access all char;
   --  subtype const_charp         is X.Strings.const_charp;
   type const_charp       is access constant char;
   nul : Interfaces.C.char renames Interfaces.C.nul;

   subtype ptrdiff_t      is Interfaces.C.ptrdiff_t;
   subtype size_t         is Interfaces.C.size_t;

   --  **************************************************
   --  Array types, moved up here so the type definitions
   --  can be shared between packages.
   --  **************************************************

   subtype char_array     is Interfaces.C.char_array;
   type unsigned_char_array is array (natural_int range <>) of unsigned_char;
   type short_array is array (natural_int range <>) of short;
   type unsigned_short_array is array (natural_int range <>) of unsigned_short;
   type int_array is array (natural_int range <>) of int;
   type unsigned_int_array is array (natural_int range <>) of unsigned_int;
   type long_array is array (natural_int range <>) of long;
   type unsigned_long_array is array (natural_int range <>) of unsigned_long;
   type float_array is array (natural_int range <>) of float;
   type double_array is array (natural_int range <>) of double;

   function "&" (S : char_array; C : char) return char_array
     renames Interfaces.C."&";

   --  *******************************************************
   --  Allocate new nul-terminated strings and return pointers
   --  *******************************************************

   --  function New_String (S: String) return charp
   --  renames X.Strings.New_String;

   --  function New_String (S: String) return const_charp
   --  renames X.Strings.New_String;

   --  *************************************************
   --  Map C untyped "void *" pointers to System.Address
   --  *************************************************

   subtype Void_Star is System.Address;
   type function_pointer is access procedure;  -- untyped

   --  *********************************************************
   --  In C, a variable-size array is declared as a[1] or
   --   a[ANYSIZE_ARRAY], where ANYSIZE_ARRAY is defined as 1.
   --  Then it is used as if it were bigger.
   --  In Ada we declare it as array (0..ANYSIZE_ARRAY) and then
   --  use the extensible array package.
   --  In C ANYSIZE_ARRAY is 1 and in Ada it is 0.
   --  *********************************************************

   ANYSIZE_ARRAY : constant := 0;                           -- winnt.h:26

   --  ****************************************************
   --  Types moved up here, to break circular dependencies,
   --  and to remove duplicate definitions:
   --  ****************************************************

   type int_access is access all int;
   type unsigned_long_access is access all unsigned_long;
   type unsigned_char_access is access all unsigned_char;
   type const_unsigned_char_access is access constant unsigned_char;
   type wchar_access is access all Interfaces.C.wchar_t;   -- wchar *
   type wchar_access_access is access all wchar_access;    -- wchar **

   --  *********************
   --  bit fields in records
   --  *********************
   type bits1   is mod 2 ** 1;  for bits1'Size use 1;
   type bits2   is mod 2 ** 2;  for bits2'Size use 2;
   type bits3   is mod 2 ** 3;  for bits3'Size use 3;
   type bits4   is mod 2 ** 4;  for bits4'Size use 4;
   type bits5   is mod 2 ** 5;  for bits5'Size use 5;
   type bits6   is mod 2 ** 6;  for bits6'Size use 6;
   type bits7   is mod 2 ** 7;  for bits7'Size use 7;
   type bits8   is mod 2 ** 8;  for bits8'Size use 8;
   type bits9   is mod 2 ** 9;  for bits9'Size use 9;
   type bits10  is mod 2 ** 10;  for bits10'Size use 10;
   type bits11  is mod 2 ** 11;  for bits11'Size use 11;
   type bits12  is mod 2 ** 12;  for bits12'Size use 12;
   type bits13  is mod 2 ** 13;  for bits13'Size use 13;
   type bits14  is mod 2 ** 14;  for bits14'Size use 14;
   type bits15  is mod 2 ** 15;  for bits15'Size use 15;
   type bits16  is mod 2 ** 16;  for bits16'Size use 16;
   type bits17  is mod 2 ** 17;  for bits17'Size use 17;
   type bits18  is mod 2 ** 18;  for bits18'Size use 18;
   type bits19  is mod 2 ** 19;  for bits19'Size use 19;
   type bits20  is mod 2 ** 20;  for bits20'Size use 20;
   type bits21  is mod 2 ** 21;  for bits21'Size use 21;
   type bits22  is mod 2 ** 22;  for bits22'Size use 22;
   type bits23  is mod 2 ** 23;  for bits23'Size use 23;
   type bits24  is mod 2 ** 24;  for bits24'Size use 24;
   type bits25  is mod 2 ** 25;  for bits25'Size use 25;
   type bits26  is mod 2 ** 26;  for bits26'Size use 26;
   type bits27  is mod 2 ** 27;  for bits27'Size use 27;
   type bits28  is mod 2 ** 28;  for bits28'Size use 28;
   type bits29  is mod 2 ** 29;  for bits29'Size use 29;
   type bits30  is mod 2 ** 30;  for bits30'Size use 30;
   type bits31  is mod 2 ** 31;  for bits31'Size use 31;
   type bits32  is mod 2 ** 32;  for bits32'Size use 32;

   --  *********************************************
   --  Support function for C macros and expressions
   --  *********************************************
   function Sizeof (Bits : Integer) return int;
   function Bool_to_Int (Val : Boolean) return int;
   procedure Call (Ignored_Function_Result : int);
   procedure Call (Ignored_Function_Result : charp);
   function To_C (C : Character) return Interfaces.C.char;

   function "+" (C : char; I : int) return char;
   pragma Inline ("+");

   function "+" (C : char; I : int) return int;
   pragma Inline ("+");

private

   pragma Inline (Sizeof);
   pragma Inline (Bool_to_Int);
   pragma Inline (Call);
   pragma Inline (To_C);

end C;
