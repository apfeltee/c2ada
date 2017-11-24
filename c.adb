with Ada.Unchecked_Conversion;

package body C is

    use Interfaces.C;

    function U is new Ada.Unchecked_Conversion (Int, Unsigned);
    function I is new Ada.Unchecked_Conversion (Unsigned, Int);

    function Bool_to_Int (Val: Boolean) return Int is
    begin
	if Val = False then return 0; else return 1; end if;
    end Bool_to_Int;

    function Sizeof (Bits: Integer) return Int is
    begin
	return Int(Bits/System.Storage_Unit);
    end Sizeof;

    procedure Call (Ignored_Function_Result: Int) is
    begin
	null;
    end Call;

    procedure Call (Ignored_Function_Result: Charp) is
    begin
	null;
    end Call;

    function "+" (C: char; I: Int) return char is
    begin
	return char'val(Int(char'pos(C)) + I);
    end "+";

    function "+" (C: char; I: Int) return Int is
    begin
	return (char'pos(C) + I);
    end "+";

    function To_C (C: Character) return Interfaces.C.Char is
    begin
	return Interfaces.C.Char'Val(Character'Pos(C));
    end To_C;

end C;
