<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE eagle SYSTEM "eagle.dtd">
<eagle version="6.3">
<drawing>
<settings>
<setting alwaysvectorfont="no"/>
<setting verticaltext="up"/>
</settings>
<grid distance="0.1" unitdist="inch" unit="inch" style="lines" multiple="1" display="no" altdistance="0.01" altunitdist="inch" altunit="inch"/>
<layers>
<layer number="1" name="Top" color="4" fill="1" visible="yes" active="no"/>
<layer number="16" name="Bottom" color="1" fill="1" visible="yes" active="no"/>
<layer number="17" name="Pads" color="2" fill="1" visible="yes" active="no"/>
<layer number="18" name="Vias" color="2" fill="1" visible="yes" active="no"/>
<layer number="19" name="Unrouted" color="6" fill="1" visible="yes" active="no"/>
<layer number="20" name="Dimension" color="15" fill="1" visible="yes" active="no"/>
<layer number="21" name="tPlace" color="7" fill="1" visible="yes" active="no"/>
<layer number="22" name="bPlace" color="7" fill="1" visible="yes" active="no"/>
<layer number="23" name="tOrigins" color="15" fill="1" visible="yes" active="no"/>
<layer number="24" name="bOrigins" color="15" fill="1" visible="yes" active="no"/>
<layer number="25" name="tNames" color="7" fill="1" visible="yes" active="no"/>
<layer number="26" name="bNames" color="7" fill="1" visible="yes" active="no"/>
<layer number="27" name="tValues" color="7" fill="1" visible="yes" active="no"/>
<layer number="28" name="bValues" color="7" fill="1" visible="yes" active="no"/>
<layer number="29" name="tStop" color="7" fill="3" visible="no" active="no"/>
<layer number="30" name="bStop" color="7" fill="6" visible="no" active="no"/>
<layer number="31" name="tCream" color="7" fill="4" visible="no" active="no"/>
<layer number="32" name="bCream" color="7" fill="5" visible="no" active="no"/>
<layer number="33" name="tFinish" color="6" fill="3" visible="no" active="no"/>
<layer number="34" name="bFinish" color="6" fill="6" visible="no" active="no"/>
<layer number="35" name="tGlue" color="7" fill="4" visible="no" active="no"/>
<layer number="36" name="bGlue" color="7" fill="5" visible="no" active="no"/>
<layer number="37" name="tTest" color="7" fill="1" visible="yes" active="no"/>
<layer number="38" name="bTest" color="7" fill="1" visible="yes" active="no"/>
<layer number="39" name="tKeepout" color="4" fill="11" visible="no" active="no"/>
<layer number="40" name="bKeepout" color="1" fill="11" visible="no" active="no"/>
<layer number="41" name="tRestrict" color="4" fill="10" visible="no" active="no"/>
<layer number="42" name="bRestrict" color="1" fill="10" visible="no" active="no"/>
<layer number="43" name="vRestrict" color="2" fill="10" visible="no" active="no"/>
<layer number="44" name="Drills" color="7" fill="1" visible="no" active="no"/>
<layer number="45" name="Holes" color="7" fill="1" visible="no" active="no"/>
<layer number="46" name="Milling" color="3" fill="1" visible="yes" active="no"/>
<layer number="47" name="Measures" color="7" fill="1" visible="yes" active="no"/>
<layer number="48" name="Document" color="7" fill="1" visible="yes" active="no"/>
<layer number="49" name="Reference" color="7" fill="1" visible="yes" active="no"/>
<layer number="51" name="tDocu" color="7" fill="1" visible="yes" active="no"/>
<layer number="52" name="bDocu" color="7" fill="1" visible="yes" active="no"/>
<layer number="91" name="Nets" color="2" fill="1" visible="yes" active="yes"/>
<layer number="92" name="Busses" color="1" fill="1" visible="yes" active="yes"/>
<layer number="93" name="Pins" color="2" fill="1" visible="no" active="yes"/>
<layer number="94" name="Symbols" color="4" fill="1" visible="yes" active="yes"/>
<layer number="95" name="Names" color="7" fill="1" visible="yes" active="yes"/>
<layer number="96" name="Values" color="7" fill="1" visible="yes" active="yes"/>
<layer number="97" name="Info" color="7" fill="1" visible="yes" active="yes"/>
<layer number="98" name="Guide" color="6" fill="1" visible="yes" active="yes"/>
</layers>
<schematic xreflabel="%F%N/%S.%C%R" xrefpart="/%S.%C%R">
<libraries>
<library name="PIN">
<packages>
<package name="1X01_1.1_UNSIGNED">
<pad name="1" x="0" y="0" drill="1.1"/>
<text x="-2.54" y="1.27" size="1.27" layer="21">&gt;NAME</text>
</package>
<package name="1X01_1.1_UNSIGNED_LONG">
<pad name="1" x="0" y="0" drill="1.1" shape="long" rot="R270"/>
<text x="-2.54" y="2.54" size="1.27" layer="21">&gt;NAME</text>
</package>
<package name="1X01_0.8_UNSIGNED_SQUARE">
<pad name="1" x="0" y="0" drill="0.8" shape="square"/>
<text x="-3.2" y="0.8" size="1.27" layer="21">&gt;NAME</text>
</package>
<package name="1X01_0.8_UNSIGNED">
<pad name="1" x="0" y="0" drill="0.8"/>
<text x="-2.54" y="1.27" size="1.27" layer="21">&gt;NAME</text>
</package>
<package name="1X01_0.8_UNSIGNED_LONG">
<pad name="1" x="0" y="0" drill="0.8" shape="long" rot="R270"/>
<text x="-2.54" y="2.54" size="1.27" layer="21">&gt;NAME</text>
</package>
<package name="1X01_1.27X0.635_PAD">
<text x="-2.54" y="1.27" size="1.27" layer="21">&gt;NAME</text>
<smd name="1" x="0" y="0" dx="1.27" dy="0.635" layer="1"/>
</package>
<package name="1X01_1X1_PAD">
<text x="-2.54" y="1.27" size="1.27" layer="21">&gt;NAME</text>
<smd name="1" x="0" y="0" dx="1" dy="1" layer="1"/>
</package>
<package name="1X01_1X2_PAD">
<text x="-2.54" y="1.27" size="1.27" layer="21">&gt;NAME</text>
<smd name="1" x="0" y="0" dx="2" dy="1" layer="1"/>
</package>
</packages>
<symbols>
<symbol name="1X01">
<wire x1="-2.54" y1="2.54" x2="-2.54" y2="-2.54" width="0.254" layer="94"/>
<wire x1="-2.54" y1="-2.54" x2="5.08" y2="-2.54" width="0.254" layer="94"/>
<wire x1="5.08" y1="-2.54" x2="5.08" y2="2.54" width="0.254" layer="94"/>
<wire x1="5.08" y1="2.54" x2="-2.54" y2="2.54" width="0.254" layer="94"/>
<text x="-2.286" y="3.302" size="1.27" layer="95">&gt;NAME</text>
<pin name="1" x="0" y="0" visible="pad" length="short" direction="pas" function="dot"/>
</symbol>
</symbols>
<devicesets>
<deviceset name="1X01">
<gates>
<gate name="G$1" symbol="1X01" x="0" y="0"/>
</gates>
<devices>
<device name="" package="1X01_1.1_UNSIGNED">
<connects>
<connect gate="G$1" pin="1" pad="1"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
<device name="LONG" package="1X01_1.1_UNSIGNED_LONG">
<connects>
<connect gate="G$1" pin="1" pad="1"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
<device name="08S" package="1X01_0.8_UNSIGNED_SQUARE">
<connects>
<connect gate="G$1" pin="1" pad="1"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
<device name="08" package="1X01_0.8_UNSIGNED">
<connects>
<connect gate="G$1" pin="1" pad="1"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
<device name="08L" package="1X01_0.8_UNSIGNED_LONG">
<connects>
<connect gate="G$1" pin="1" pad="1"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
<device name="P" package="1X01_1.27X0.635_PAD">
<connects>
<connect gate="G$1" pin="1" pad="1"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
<device name="P1" package="1X01_1X1_PAD">
<connects>
<connect gate="G$1" pin="1" pad="1"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
<device name="P2" package="1X01_1X2_PAD">
<connects>
<connect gate="G$1" pin="1" pad="1"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
</devicesets>
</library>
</libraries>
<attributes>
</attributes>
<variantdefs>
</variantdefs>
<classes>
<class number="0" name="default" width="0" drill="0">
</class>
</classes>
<parts>
<part name="HEATER1" library="PIN" deviceset="1X01" device="P2" value="1X01P2"/>
<part name="HEATER2" library="PIN" deviceset="1X01" device="P2" value="1X01P2"/>
<part name="THERM1" library="PIN" deviceset="1X01" device="P2" value="1X01P2"/>
<part name="THERM2" library="PIN" deviceset="1X01" device="P2" value="1X01P2"/>
</parts>
<sheets>
<sheet>
<plain>
</plain>
<instances>
<instance part="HEATER1" gate="G$1" x="40.64" y="78.74"/>
<instance part="HEATER2" gate="G$1" x="40.64" y="71.12"/>
<instance part="THERM1" gate="G$1" x="40.64" y="63.5"/>
<instance part="THERM2" gate="G$1" x="40.64" y="55.88"/>
</instances>
<busses>
</busses>
<nets>
<net name="HEAT_LINE" class="0">
<segment>
<pinref part="HEATER1" gate="G$1" pin="1"/>
<wire x1="40.64" y1="78.74" x2="33.02" y2="78.74" width="0.1524" layer="91"/>
<wire x1="33.02" y1="78.74" x2="33.02" y2="71.12" width="0.1524" layer="91"/>
<pinref part="HEATER2" gate="G$1" pin="1"/>
<wire x1="33.02" y1="71.12" x2="40.64" y2="71.12" width="0.1524" layer="91"/>
</segment>
</net>
</nets>
</sheet>
</sheets>
</schematic>
</drawing>
</eagle>
