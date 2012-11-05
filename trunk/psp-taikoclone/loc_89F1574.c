if (tag == 0xFF00) {//loc_89F25E4
    somehow returns0;
} else if (tag > 0xF00D) {//loc_89F15FC
    if (tag > 0xF022) {//loc_89F16C8
        if (tag != 0xF103) {//loc_89F168C
            somehow returns;
        } else {//loc_89F16D0
            //又来一长串
        }
    } else if (tag <= 0xF021) {//loc_89F168C
        somehow returns;
    } else {//loc_89F1618
        //又来一长串,其实就是tag == 0xF022
    }
} else if (tag >= 0x28) {//loc_89F15D4
    if (tag <= 0xF000) {//loc_89F1688
        somehow returns;
    } else {
        tag -= 0xF001;
        do something;
    }
} else if (tag < 7) {//loc_89F1688
    somehow_returns;
} else {    //0x7 ~ 0x27, loc_89F1C2C
    tag -= 7;
    do something;
}

if (tag == 0xFF00) {//end tag?
    return;
}

//0x0 ~ 0x6
//0x7 ~ 0x27    do
//0x28 ~ 0xF000
//0xF001 ~ 0xF00D do
//0xF00E ~ 0xF021 + 0xF103
//0xF022	define sprite!
//		+0x4(2)		CharacterID
//0xF023 ~ ? -0xF103

//0x0004 似乎是place object //0x0001 似乎是show frame // // // 
//flag      handler 
//0xF001    0x89f170c   //build some table, s7->0xC->0x44->0xC,申请好内存,到0x38,再来一片，对应每个符号查表地址到3C
//0xF002    0x89f188c   //s7->0xC->0x44->0x10,s7->0xC->0x44->0x40 
//0xF003    0x89f18c8   //s7->0xC->0x44->0x14,s7->0xC->0x44->0x44 
//      color transform!
//0xF004    0x89f1904   //s7->0xC->0x44->0x1C,s7->0xC->0x44->0x4C 
//      +0x0(2)     tag_type
//      +0x2(2)     tag_size
//      +0x4(4)     元件大小对应表，貌似从未被读取过
//      +0x8(+)
//
//0xF005    0x89f1940   //s7->0xC->0x44->0x20,(if!0),申请内存，s7...0x50，展开成4字节对齐 actionscript?
//      +0x4(4)     actionscript个数！
//      +0x8~??     bytecodelength + bytes(4字节对齐)
//
//0xF007    0x89f1a14   //s7->0xC->0x44->0x24,(if s7->0x0->0x8 func? and !0)s7->0xC->0x44->0x54, fill this mem?  
//0xF008    0x89f1ae4   //s7->0xC->0x44->0x28,(if s7->0x0->0x8 func? and !0)s7->0xC->0x44->0x58, fill this mem?  
//0xF009    0x89f1bb4   //s7->0xC->0x44->0x2C,s7->0xC->0x44->0x5C 
//0xF00A    0x89f1bf0   //s7->0xC->0x44->0x30,s7->0xC->0x44->0x60 
//0xF00B    0x89f1c68   //skip 
//0xF00C    0x89f1c78   //blkptr ---> s7->0xC->0x44->0x68, blk_off0xE --> s7->0xC->0x44->0x6C, 申请内存，-->s7->0xC->0x44->0x84,F022块索引??
//		+0xA(2)			MaxCharacterID
//
//	X->0x84:
//		CharacterID		tagTypeAndSize
//  X->0x88:
//		CharacterID		tagF022ptr, tagF023ptr
//0xF00D    0x89f1ce0   //0ff0x4半字 --> s7->0xC->0x44->0x70, 0ff0x6半字 -->s7->0xC->0x44->0x74,0x8-->0x78,0xA->0x7c,0xC->0x80,以第一个半字申请内存*8，->0x88,第二个半字申请内存*8->0x8c,第三个半字申请内存*16->0x90,第四个半字申请内存*8->0x94,第6个半字申请内存*4->0x98 //          第一半字内存块 f022-f023块对应？,猜想是f022对应的第一块f023, 
//		+0x0(2)		tag_type
//		+0x2(2)		tag_size
//		+0x8(2)		cnt_tag0027_followed
//		
//0xF103    directjmp   //s7->0xC->0x44->0x18,s7->0xC->0x44->0x48

//0x000A    0x89f1c2c   //s7->0xC->0x44->0x34,s7->0xC->0x44->0x64
//0xF022    0x89f1618   //build s7->0xC->0x44->0x88, s7->0xC->0x44->0x84块
//      +0x0(2)     tag_type
//      +0x2(2)     tag_size
//      +0x4(2)     ??(某id性质的东西)
//		+0x8(2)	    元件的大小？(还缺一个查询表)
//		+0xa(2)		cnt_tagF023_followed
//
//		example:
TITLE_ROGO_00.LM
0: 512*256
1: 256*32
2: 512*16
3:
4: 256*128
5: 256*256
6: 64*64
7: 64*128

PLAYER_SELECT_BG.LM
0:
1: 256*256
2: 128*256
3: 128*128

ATTRACT_A0050.LM
0: 512*512
1: 8*8

PLAYER_SELECT.LM
0: 64*256
1: 4*4
2: 32*32
//		
//
//0xF023~? -0xF103 s7->0xC->0x44->0x18,//s7->0xC->0x44->0x48,目测没有其他值，只有F023
//		
//
//      +0x0(2)     tag_type
//      +0x2(2)     tag_size
//      +0x4(4)     
//      +0x8(4)     sub8964278
//      +0xc(4)
//      +0x10(4)    sub8964278
//      所有值的计算，都在sub8964278
//
//最复杂的tag,明天再搞，估计大工作量
//0x0027    0x89f21c0   s7->0xC->0x44->0x90 + 偏移 * 16 偏移=栈上某计数(tag0027计数)
//                      +0x0(4)     tag0027addr
//                      +0x4(4)     按size=tag0027->0xA(2) *4申请内存，保存所有tag002b的addr
//                      +0x8(4)     按size=tag0027->0xC(2) *4申请内存，保存所有tag0001的addr
//                      +0xC(4)     按size=tag0027->0xE(2) *4申请内存, 保存所有tagf105的addr
//
//
//                      s7->0xC->0x44->0x84 + 偏移 * 4  偏移=tag0027->0x4(2)
//                      +0x0(2)     val:0027
//                      +0x2(2)     tag0027计数
//                      (感觉处理成跟F022块类似的)
//
//tag0027
//      +0x0(2)     tag_type
//      +0x2(2)     tag_size(不含tag_type和tag_size)
//      +0x4(2)     CharacterID
//      +0xA(2)     cnt_tag002b_followed
//      +0xC(2)     cnt_tag0001_followed
//      +0xE(2)     cnt_tagf105_followed
//      +0x10(4)    cnt_kind_of_Character?
//
//tag0001
//		+0x4(2)		帧序号
//      +0x6(2)     接下来还有几个tag0004	
//
//tag0004	PlaceObject
//		+0x4(2)		CharacterID
//		+0xa(2)     name_idx(符号表)
//		+0xc(2)		flags(1=PlaceFlagHasCharacter, 2=PlaceFlagMove)
//		+0x10(2)	depth
//		+0x12(2)	instanceID
//		+0x1a(2)    colorMulIdx
//		+0x1c(2)	colorAddIdx
//      +0x20(2)    ??决定跳过几个tag
//
//tag0005	RemoveObject
//		+0x0(2)		tag_type
//		+0x2(2)		tag_size
//		+0x6(2)		depth
//
//		
//tagf105	定义关键帧参考
//      +0x6(2)     后面还有几个tag0004
//
//tagf014   执行ClipAction,target为前一个PlaceObject的对象
//
//tag002b	定义一个命名标签
//		+0x0(2)		tag_type
//		+0x2(2)		tag_size
//		+0x4(2)		name_idx
//		+0x6(2)		frame_idx
//
//关于CharacterID:
// 可以产生CharacterID的tag：
// F022
// 0027
// tag0027可以引用F022
//
// script解析:
// 0x8a30e80
//
//0x96:set variable?
//0x2:接下来的长度
//0x8:?
//0x8:符号表的索引

//0x96:


//as@PLAYER_SELECT_BG.LM+0x1f8(0x25)

//ActionPush type:constant8(0x8) value:0x8			//8

//ActionGetVariable									//"this"	

//ActionPush type:constant8(0x8) value:0x9			//"this"
//			 type:constant8(0x8) value:0x8			//9
													//8
			 
//ActionGetVariable									//"this"
													//9
													//"this"

//ActionPush type:constant8(0x8) value:0x9			//"this"
													//9
													//"this"
													//9

//ActionGetMember									//"this"
													//9
													//v0	(v0=this._rotation)

//ActionPush type:double(0x6) value=\x9a\x99\x99\x99\x99\x99\xc9\x3f=0.2
													//"this"
													//9
													//v0
													//0.2

//ActionAdd2										//"this"
													//9
													//v1 (v1=v0+0.2)

//ActionSetMember									//NULL	(this._rotation=v1)

//ascode:
//	this._rotation += 0.2

					
//as@PLAYER_SELECT_BG.LM+0x250(0x61)

//ActionPush type:constant8(0x8) value:0xA			//0xA

//ActionDefineFunction name:anonymous(0x0), numParam:0x0, codeSize:0x50

		//ActionPush type:constant8(0x8) value:0x3	//0x3
		
		//ActionGetVariable							//"mc_bg"
		
		//ActionPush type:constant8(0x8) value:0xB	//"mc_bg"
		//			 type:constant8(0x8) value:0x3	//0xB
													//0x3

		//ActionGetVariable							//"mc_bg"
													//0xB
													//"mc_bg"
													
		//ActionPush type:constant8(0x8) value:0xB	//"mc_bg"
													//0xB
													//"mc_Bg"
													//0xB
		
		//ActionGetMember							//"mc_bg"
													//0xB
													//v0 (v0=mc_bg._x)
		
		//ActionPush type:double(0x6) value:0.4		//"mc_bg"
													//0xB
													//v0
													//0.4
		
		//ActionSubstract							//"mc_bg"
													//0xB
													//v1	(v0-0.4)
		
		//ActionSetMember							//NULL	(mc_bg._x = v1)
		

													
		//ActionPush 								//0x3

		//ActionGetVariable							//"mc_bg"
													
		//ActionPush								//"mc_bg"
													//0xB
													
		//ActionGetMember							//v0 (v0=mc_bg._x)

		//ActionPush								//v0
													//-480
													
		//ActionGreater								//v1 (v1=(v0>-480))
													
		//ActionNot									//v2 (v2=!v1=(v0<=-480)
													
		//ActionNot									//v1
													
		//ActionIf									
		
		//ActionNot									

		//ActionPush								//0x3
		
		//ActionGetVariable							//"mc_bg"
		
		//ActionPush								//"mc_bg"
													//0xB
													//0(integer)
													
		//ActionSetMember							//mc._x=0
					
//ascode:		
//onEnterFrame = function() {
//	mc_bg._x -= 0.4
//	if (mc_bg._x <= -480) {
//		mc_bg._x = 0
//	}
//}
//
//ascode@ONP_FLY_DON_D.LM+0x52c(0x3)
//stop()
//
//ascode@ONP_FLY_DON_D.LM+0x534(0x1A)
//ExternalInterface.call("FSCommand:event", "end")
//stop()
//
//
