\def\PROGb{% G:\Master Project\doc\ReportFinal\curvature.tex:78
\begin{@CPP}%
\NL{1}
\KW{int}\SP \ID{RayIntestectsBVH}(\ID{vec3}\SP \ID{origine},\SP \ID{vec3}\SP \ID{dir})\{\NL{2}
\SP \TAB \ID{vec4}\SP \ID{bottomC},\SP \ID{topC};\NL{3}
\SP \TAB \ID{top}\SP =\SP 0;\NL{4}
\SP \TAB \KW{int}\SP \ID{node}\SP =\SP 0,\SP \ID{c1}\SP =0,\SP \ID{c2}\SP =\SP 0;\NL{5}
\EMPTYLINE{6}\SP \TAB \NOTE push\SP the\SP BVH\SP top\SP node\SP on\SP the\SP stack\SP \SP \SP \SP \TAB \ENDNOTE \NL{7}
\SP \TAB \ID{push}(\ID{node});\NL{8}
\SP \TAB \KW{while}(\ID{top}\SP \textgreater \SP 0)\{\NL{9}
\SP \TAB \SP \SP \SP \SP \TAB \NOTE First\SP non\SP expanded\SP node\ENDNOTE \NL{10}
\SP \TAB \SP \SP \SP \SP \TAB \ID{node}\SP =\SP \ID{pop}();\NL{11}
\EMPTYLINE{12}\SP \TAB \SP \SP \SP \SP \TAB \NOTE Load\SP the\SP AABB\SP of\SP the\SP node\ENDNOTE \NL{13}
\SP \TAB \SP \SP \SP \SP \TAB \ID{bottomC}\SP =\SP \ID{nodeBottom}(\ID{node});\NL{14}
\SP \TAB \SP \SP \SP \SP \TAB \ID{topC}\SP =\SP \ID{nodeTop}(\ID{node});\NL{15}
\EMPTYLINE{16}\SP \TAB \SP \SP \SP \SP \TAB \NOTE If\SP this\SP is\SP a\SP leaf\SP check\SP for\SP intersection\SP with\SP the\SP surface\ENDNOTE \NL{17}
\SP \TAB \SP \SP \SP \SP \TAB \KW{if}(\ID{bottomC}.\ID{w}\SP !=\SP {-}\relax 1.0\SP \textbar \textbar \SP \ID{topC}.\ID{w}\SP !=\SP {-}\relax 1.0)\{\NL{18}
\SP \TAB \SP \SP \SP \SP \TAB \SP \SP \SP \SP \TAB \NOTE This\SP test\SP loads\SP the\SP primitives\SP if\SP the\SP node\SP and\SP tries\SP to\SP find\SP a\SP point\SP inside\SP the\SP surface.\ENDNOTE \NL{19}
\SP \TAB \SP \SP \SP \SP \TAB \SP \SP \SP \SP \TAB \NOTE The\SP test\SP is\SP positive\SP if\SP such\SP a\SP point\SP is\SP found.\ENDNOTE \NL{20}
\SP \TAB \SP \SP \SP \SP \TAB \SP \SP \SP \SP \TAB \KW{if}(\ID{rayIntersectsSurfaceInNode}(\ID{origine},\SP \ID{dir},\SP \ID{node}))\SP \KW{return}\SP \ID{node};\NL{21}
\SP \TAB \SP \SP \SP \SP \TAB \}\KW{else}\{\NL{22}
\SP \TAB \SP \SP \SP \SP \TAB \SP \SP \SP \SP \TAB \NOTE Otherwise\SP expand\ENDNOTE \NL{23}
\SP \TAB \SP \SP \SP \SP \TAB \SP \SP \SP \SP \TAB \ID{c1}\SP =\SP \ID{child1}(\ID{node});\NL{24}
\SP \TAB \SP \SP \SP \SP \TAB \SP \SP \SP \SP \TAB \ID{c2}\SP =\SP \ID{child2}(\ID{node});\NL{25}
\SP \TAB \SP \SP \SP \SP \TAB \SP \SP \SP \SP \TAB \NOTE Distances\SP from\SP the\SP eye\SP to\SP the\SP AABBS\SP intersection\ENDNOTE \NL{26}
\SP \TAB \SP \SP \SP \SP \TAB \SP \SP \SP \SP \TAB \KW{float}\SP \ID{d1};\NL{27}
\SP \TAB \SP \SP \SP \SP \TAB \SP \SP \SP \SP \TAB \KW{float}\SP \ID{d2};\NL{28}
\EMPTYLINE{29}\SP \TAB \SP \SP \SP \SP \TAB \SP \SP \SP \SP \TAB \NOTE Test\SP c1\SP for\SP intersection\ENDNOTE \NL{30}
\SP \TAB \SP \SP \SP \SP \TAB \SP \SP \SP \SP \TAB \ID{bottomC}\SP =\SP \ID{nodeBottom}(\ID{c1});\NL{31}
\SP \TAB \SP \SP \SP \SP \TAB \SP \SP \SP \SP \TAB \ID{topC}\SP =\SP \ID{nodeTop}(\ID{c1});\NL{32}
\SP \TAB \SP \SP \SP \SP \TAB \SP \SP \SP \SP \TAB \KW{bool}\SP \ID{c1Intersects}\SP =\SP \ID{RayIntestectsAABB}(\ID{origine},\SP \ID{dir},\SP \ID{bottomC}.\ID{xyz},\SP \ID{topC}.\ID{xyz},\SP \ID{d1});\NL{33}
\SP \TAB \SP \SP \SP \SP \TAB \SP \SP \SP \SP \TAB \NOTE Test\SP c2\ENDNOTE \NL{34}
\SP \TAB \SP \SP \SP \SP \TAB \SP \SP \SP \SP \TAB \ID{bottomC}\SP =\SP \ID{nodeBottom}(\ID{c2});\NL{35}
\SP \TAB \SP \SP \SP \SP \TAB \SP \SP \SP \SP \TAB \ID{topC}\SP =\SP \ID{nodeTop}(\ID{c2});\NL{36}
\SP \TAB \SP \SP \SP \SP \TAB \SP \SP \SP \SP \TAB \KW{bool}\SP \ID{c2Intersects}\SP =\SP \ID{RayIntestectsAABB}(\ID{origine},\SP \ID{dir},\SP \ID{bottomC}.\ID{xyz},\SP \ID{topC}.\ID{xyz},\SP \ID{d2});\NL{37}
\EMPTYLINE{38}\SP \TAB \SP \SP \SP \SP \TAB \SP \SP \SP \SP \TAB \NOTE Push\SP the\SP children\SP if\SP their\SP intersect\SP by\SP order\SP of\SP distance\SP to\SP the\SP eye\ENDNOTE \NL{39}
\SP \TAB \SP \SP \SP \SP \TAB \SP \SP \SP \SP \TAB \KW{if}(\SP \ID{c1Intersects}\SP )\{\NL{40}
\SP \TAB \SP \SP \SP \SP \TAB \SP \SP \SP \SP \TAB \SP \SP \SP \SP \TAB \KW{if}(\SP \ID{c2Intersects}\SP )\{\NL{41}
\SP \TAB \SP \SP \SP \SP \TAB \SP \SP \SP \SP \TAB \SP \SP \SP \SP \TAB \SP \SP \SP \SP \TAB \KW{if}(\SP \ID{d1}\SP \textless =\SP \ID{d2}\SP )\{\NL{42}
\SP \TAB \SP \SP \SP \SP \TAB \SP \SP \SP \SP \TAB \SP \SP \SP \SP \TAB \SP \SP \SP \SP \TAB \SP \SP \SP \SP \TAB \ID{push}(\ID{c2});\NL{43}
\SP \TAB \SP \SP \SP \SP \TAB \SP \SP \SP \SP \TAB \SP \SP \SP \SP \TAB \SP \SP \SP \SP \TAB \SP \SP \SP \SP \TAB \ID{push}(\ID{c1});\NL{44}
\SP \TAB \SP \SP \SP \SP \TAB \SP \SP \SP \SP \TAB \SP \SP \SP \SP \TAB \SP \SP \SP \SP \TAB \}\KW{else}\{\NL{45}
\SP \TAB \SP \SP \SP \SP \TAB \SP \SP \SP \SP \TAB \SP \SP \SP \SP \TAB \SP \SP \SP \SP \TAB \SP \SP \SP \SP \TAB \ID{push}(\ID{c1});\NL{46}
\SP \TAB \SP \SP \SP \SP \TAB \SP \SP \SP \SP \TAB \SP \SP \SP \SP \TAB \SP \SP \SP \SP \TAB \SP \SP \SP \SP \TAB \ID{push}(\ID{c2});\NL{47}
\SP \TAB \SP \SP \SP \SP \TAB \SP \SP \SP \SP \TAB \SP \SP \SP \SP \TAB \SP \SP \SP \SP \TAB \}\NL{48}
\SP \TAB \SP \SP \SP \SP \TAB \SP \SP \SP \SP \TAB \SP \SP \SP \SP \TAB \}\KW{else}\NL{49}
\SP \TAB \SP \SP \SP \SP \TAB \SP \SP \SP \SP \TAB \SP \SP \SP \SP \TAB \SP \SP \SP \SP \TAB \ID{push}(\ID{c1});\NL{50}
\SP \TAB \SP \SP \SP \SP \TAB \SP \SP \SP \SP \TAB \}\KW{else}\SP \KW{if}(\SP \ID{c2Intersects}\SP )\NL{51}
\SP \TAB \SP \SP \SP \SP \TAB \SP \SP \SP \SP \TAB \SP \SP \SP \SP \TAB \ID{push}(\ID{c2});\NL{52}
\SP \TAB \SP \SP \SP \SP \TAB \}\NL{53}
\SP \TAB \}\NL{54}
\SP \TAB \KW{return}\SP {-}\relax 1;\NL{55}
\}\NL{56}
\end{@CPP}%
}%
