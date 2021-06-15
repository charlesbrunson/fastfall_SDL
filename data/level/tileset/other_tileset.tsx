<?xml version="1.0" encoding="UTF-8"?>
<tileset version="1.5" tiledversion="1.6.0" name="other_tileset" tilewidth="16" tileheight="16" tilecount="1" columns="1">
 <properties>
  <property name="anim" value="&lt;anim ms=&quot;200&quot; nextx=&quot;2&quot; nexty=&quot;4&quot; tileset=&quot;tile_test&quot;/&gt;"/>
  <property name="shape" value="solid"/>
 </properties>
 <image source="tile_test2.png" width="16" height="16"/>
 <tile id="0">
  <properties>
   <property name="logic" value="anim"/>
   <property name="logic_arg" value="50"/>
   <property name="next_tileset" value="tile_test"/>
   <property name="next_x" type="int" value="3"/>
   <property name="next_y" value="4"/>
   <property name="shape" value="solid"/>
  </properties>
 </tile>
</tileset>
