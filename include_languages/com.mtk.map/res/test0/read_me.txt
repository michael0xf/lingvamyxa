The first code examples see in '/mtk/map'. 
These are first examples of data file for simple uses of overlappingTreesPlainMap library. Source file "test.mtk" is normal XML-similar hierarchy, but in derived file "snapshot..." we can find blocks such as 
<line lineY="0">
	<word wordX="0">
		snapShot_Mon_Jan_23_0
</line>
		<line lineY="15">
		1-55-06_EET_2017.xml
	</word>
		</line>
The derived file "snapshot..." was fully builded by factory, which implements com.mtk.map.IFactory, from source file and we can restore application state by this snapshot.
