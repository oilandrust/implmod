\def\PROGb{% main.tex:95
\begin{@CPP}%
\NL{1}
\SP \TAB \KW{static}\SP \ID{GLuint}\SP \ID{buffers}[2];\NL{2}
\SP \TAB \KW{static}\SP \KW{bool}\SP \ID{buff\_obj}=\KW{false};\NL{3}
\SP \TAB \KW{static}\SP \KW{int}\SP \ID{num\_vertices};\NL{4}
\SP \TAB \KW{if}(!\ID{buff\_obj})\{\NL{5}
\SP \TAB \SP \SP \SP \SP \TAB \ID{buff\_obj}\SP =\SP \KW{true};\NL{6}
\SP \TAB \SP \SP \SP \SP \TAB \ID{glGenBuffers}(2,\SP \ID{buffers});\NL{7}
\SP \TAB \SP \SP \SP \SP \TAB \ID{vector}\textless \ID{Vec3f}\textgreater \SP \ID{vertices};\NL{8}
\SP \TAB \SP \SP \SP \SP \TAB \ID{vector}\textless \ID{Vec3f}\textgreater \SP \ID{normals};\NL{9}
\EMPTYLINE{10}\SP \TAB \SP \SP \SP \SP \TAB \NOTE Preparing\SP the\SP datas\SP to\SP copy\SP in\SP the\SP vertex\SP array\ENDNOTE \NL{11}
\SP \TAB \SP \SP \SP \SP \TAB \KW{for}(\KW{int}\SP \ID{j}={-}\relax 30;\ID{j}\textless 30;++\ID{j})\{\NL{12}
\SP \TAB \SP \SP \SP \SP \TAB \SP \SP \SP \SP \TAB \NOTE repeating\SP first\SP vertex\SP of\SP the\SP row\ENDNOTE \NL{13}
\SP \TAB \SP \SP \SP \SP \TAB \SP \SP \SP \SP \TAB \ID{vertices}.\ID{push\_back}(\ID{Vec3f}({-}\relax 30,\ID{j}+1,\ID{terra}.\ID{height}({-}\relax 30,\ID{j}+1)));\NL{14}
\SP \TAB \SP \SP \SP \SP \TAB \SP \SP \SP \SP \TAB \ID{normals}.\ID{push\_back}(\ID{terra}.\ID{normal}({-}\relax 30,\ID{j}+1));\NL{15}
\EMPTYLINE{16}\SP \TAB \SP \SP \SP \SP \TAB \SP \SP \SP \SP \TAB \KW{for}(\KW{int}\SP \ID{i}={-}\relax 30;\ID{i}\textless 30;++\ID{i})\{\SP \SP \SP \TAB \NL{17}
\SP \TAB \SP \SP \SP \SP \TAB \SP \SP \SP \SP \TAB \SP \SP \SP \SP \TAB \ID{normals}.\ID{push\_back}(\ID{terra}.\ID{normal}(\ID{i},\ID{j}+1));\NL{18}
\SP \TAB \SP \SP \SP \SP \TAB \SP \SP \SP \SP \TAB \SP \SP \SP \SP \TAB \ID{vertices}.\ID{push\_back}(\ID{Vec3f}(\ID{i},\ID{j}+1,\ID{terra}.\ID{height}(\ID{i},\ID{j}+1)));\NL{19}
\SP \TAB \SP \SP \SP \SP \TAB \SP \SP \SP \SP \TAB \SP \SP \SP \SP \TAB \ID{normals}.\ID{push\_back}(\ID{terra}.\ID{normal}(\ID{i},\ID{j}));\NL{20}
\SP \TAB \SP \SP \SP \SP \TAB \SP \SP \SP \SP \TAB \SP \SP \SP \SP \TAB \ID{vertices}.\ID{push\_back}(\ID{Vec3f}(\ID{i},\ID{j},\ID{terra}.\ID{height}(\ID{i},\ID{j})));\NL{21}
\SP \TAB \SP \SP \SP \SP \TAB \SP \SP \SP \SP \TAB \}\NL{22}
\SP \TAB \SP \SP \SP \SP \TAB \SP \SP \SP \SP \TAB \NOTE repeating\SP last\SP vertex\SP of\SP the\SP row\ENDNOTE \NL{23}
\SP \TAB \SP \SP \SP \SP \TAB \SP \SP \SP \SP \TAB \ID{vertices}.\ID{push\_back}(\ID{Vec3f}(29,\ID{j},\ID{terra}.\ID{height}(29,\ID{j})));\NL{24}
\SP \TAB \SP \SP \SP \SP \TAB \SP \SP \SP \SP \TAB \ID{normals}.\ID{push\_back}(\ID{terra}.\ID{normal}(29,\ID{j}));\SP \SP \SP \SP \TAB \NL{25}
\SP \TAB \SP \SP \SP \SP \TAB \}\NL{26}
\SP \TAB \SP \SP \SP \SP \TAB \ID{num\_vertices}\SP =\SP \ID{vertices}.\ID{size}();\NL{27}
\EMPTYLINE{28}\SP \TAB \SP \SP \SP \SP \TAB \NOTE setting\SP up\SP the\SP vertex\SP buffer\SP and\SP copying\SP datas\ENDNOTE \NL{29}
\SP \TAB \SP \SP \SP \SP \TAB \ID{glBindBuffer}(\ID{GL\_ARRAY\_BUFFER},\SP \ID{buffers}[0]);\NL{30}
\SP \TAB \SP \SP \SP \SP \TAB \ID{glBufferData}(\ID{GL\_ARRAY\_BUFFER},\SP \ID{vertices}.\ID{size}()*\KW{sizeof}(\ID{Vec3f}),\NL{31}
\SP \TAB \SP \SP \SP \SP \TAB \SP \SP \SP \SP \TAB \&\ID{vertices}[0],\SP \ID{GL\_STATIC\_DRAW});\NL{32}
\SP \TAB \SP \SP \SP \SP \TAB \ID{glVertexPointer}(3,\SP \ID{GL\_FLOAT},\SP 0,\SP 0);\NL{33}
\SP \TAB \SP \SP \SP \SP \TAB \NOTE setting\SP up\SP the\SP normal\SP buffer\SP and\SP copying\SP datas\ENDNOTE \NL{34}
\SP \TAB \SP \SP \SP \SP \TAB \ID{glBindBuffer}(\ID{GL\_ARRAY\_BUFFER},\SP \ID{buffers}[1]);\NL{35}
\SP \TAB \SP \SP \SP \SP \TAB \ID{glBufferData}(\ID{GL\_ARRAY\_BUFFER},\SP \ID{normals}.\ID{size}()*\KW{sizeof}(\ID{Vec3f}),\NL{36}
\SP \TAB \SP \SP \SP \SP \TAB \SP \SP \SP \SP \TAB \&\ID{normals}[0],\SP \ID{GL\_STATIC\_DRAW});\NL{37}
\SP \TAB \SP \SP \SP \SP \TAB \ID{glNormalPointer}(\ID{GL\_FLOAT},\SP 0,\SP 0);\NL{38}
\SP \TAB \}\NL{39}
\end{@CPP}%
}%
\def\PROGc{% main.tex:144
\begin{@CPP}%
\NL{1}
\SP \TAB \ID{glEnableClientState}(\ID{GL\_VERTEX\_ARRAY});\NL{2}
\SP \TAB \ID{glEnableClientState}(\ID{GL\_NORMAL\_ARRAY});\NL{3}
\SP \TAB \ID{glDrawArrays}(\ID{GL\_TRIANGLE\_STRIP},\SP 0,\SP \ID{num\_vertices});\NL{4}
\SP \TAB \ID{glDisableClientState}(\ID{GL\_VERTEX\_ARRAY});\NL{5}
\SP \TAB \ID{glDisableClientState}(\ID{GL\_NORMAL\_ARRAY});\NL{6}
\end{@CPP}%
}%
