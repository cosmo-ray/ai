function ai_bad_mob0_callback(ai, mob)
   ai = Entity.wrapp(ai)
   mob = Entity.wrapp(mob)
   local pos = mob[1]
   local txt = ai.text
   local l_str = txt[ywPosY(pos) + 3]:to_string()
   local space_char = string.byte(" ") -- SPACEEEEEEE
   local dir = mob[2]
   local to_add = -1
   local other_dir = 0
   local check_pos = ywPosX(pos)

   if dir:to_int() ~= 1 then
      check_pos = ywPosX(pos) + 7
      other_dir = 1
      to_add = 1
   end

   print(l_str)
   if space_char == string.byte(l_str, check_pos) then
      yeSetInt(dir, other_dir)
   end

   ywPosAdd(pos, to_add, 0);
end

function ai_bat_callback(ai, mob)
   ai = Entity.wrapp(ai)
   mob = Entity.wrapp(mob)
   local pos = mob[1]
   local txt = ai.text
   local l_str = txt[ywPosY(pos) + 2]:to_string()
   local eq_char = string.byte("=")
   local pipe_char = string.byte("|")
   local dir = mob[2]
   local to_add = 1
   local other_dir = 0
   local check_pos = ywPosX(pos)

   if dir:to_int() == 1 then
      check_pos = ywPosX(pos) + 4
   else
      other_dir = 1
      to_add = -1
   end
   if eq_char == string.byte(l_str, check_pos) or
   pipe_char == string.byte(l_str, check_pos) then
      yeSetInt(dir, other_dir)
   end
   ywPosAdd(pos, to_add, 0);
end
