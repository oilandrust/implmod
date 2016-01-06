\def\PROGb{% S:\MasterProject\doc\acmsiggraph\template.tex:388
\begin{@CPP}%
\NL{1}
\NOTE Construct\SP ray\SP from\SP eye\SP to\SP fragment\ENDNOTE \NL{2}
\ID{vec3}\SP \ID{rayDir}\SP =\SP \ID{normalize}(\ID{worldPosition}{-}\relax \ID{cameraPos});\NL{3}
\ID{vec3}\SP \ID{ray}\SP =\SP \ID{worldPosition};\NL{4}
\EMPTYLINE{5}\KW{int}\SP \ID{steps}\SP =\SP 0;\SP \SP \TAB \NL{6}
\KW{float}\SP \ID{value}\SP =\SP \ID{fieldFunction}(\ID{ray});\NL{7}
\EMPTYLINE{8}\KW{while}(\SP \ID{value}\SP \textgreater \SP 0\SP \&\&\SP \ID{steps}\SP \textless \SP \ID{maxSteps}\SP )\{\NL{9}
\SP \TAB \ID{ray}\SP +=\SP \ID{stepLength}*\ID{rayDir};\NL{10}
\SP \TAB \ID{steps}++;\NL{11}
\EMPTYLINE{12}\SP \TAB \ID{value}\SP =\SP \ID{fieldFunction}(\ID{ray});\NL{13}
\}\NL{14}
\EMPTYLINE{15}\end{@CPP}%
}%
\def\PROGc{% S:\MasterProject\doc\acmsiggraph\template.tex:422
\begin{@CPP}%
\NL{1}
\ID{vec3}\SP \ID{hi};\SP \NOTE A\SP point\SP inside\SP the\SP surface\ENDNOTE \NL{2}
\ID{vec3}\SP \ID{low};\SP \NOTE A\SP point\SP outside\SP the\SP surface\ENDNOTE \NL{3}
\EMPTYLINE{4}\ID{vec3}\SP \ID{mid}\SP =\SP 0.5(\ID{hi}+\ID{low});\NL{5}
\KW{int}\SP \ID{steps}\SP =\SP 0;\NL{6}
\KW{float}\SP \ID{vmid}\SP =\SP \ID{fieldFunction}(\ID{mid});\NL{7}
\EMPTYLINE{8}\KW{while}(\ID{steps}\SP \textless \SP \ID{maxSteps}\SP \&\&\SP \ID{abs}(\ID{vmid})\SP \textgreater \SP \ID{eps})\NL{9}
\{\NL{10}
\SP \TAB \ID{steps}++;\NL{11}
\SP \TAB \ID{mid}\SP =\SP 0.5*(\ID{low}+\ID{hi});\NL{12}
\SP \TAB \KW{float}\SP \ID{vmid}\SP =\SP \ID{fieldFunction}(\ID{mid});\NL{13}
\EMPTYLINE{14}\SP \TAB \KW{if}(\ID{vmid}\SP \textless \SP 0)\NL{15}
\SP \TAB \SP \SP \SP \SP \TAB \ID{low}\SP =\SP \ID{mid};\NL{16}
\SP \TAB \KW{else}\NL{17}
\SP \TAB \SP \SP \SP \SP \TAB \ID{hi}\SP =\SP \ID{mid};\NL{18}
\}\SP \SP \SP \SP \TAB \NL{19}
\ID{vec3}\SP \ID{intersection}\SP =\SP \ID{mid};\NL{20}
\EMPTYLINE{21}\end{@CPP}%
}%
\def\PROGd{% S:\MasterProject\doc\acmsiggraph\template.tex:598
\begin{@CPP}%
\NL{1}
\EMPTYLINE{2}\ID{vec3}\SP \ID{pointOnSurface};\NL{3}
\ID{vec3}\SP \ID{restPosition}\SP =\SP \ID{vec3}(0,0,0);\SP \SP \TAB \NL{4}
\EMPTYLINE{5}\KW{for}(\KW{int}\SP \ID{bone}\SP =\SP 0;\SP \ID{bone}\SP \textless \SP \SP \ID{nbBones};\SP \ID{bone}++)\NL{6}
\{\NL{7}
\SP \TAB \NOTE compute\SP fragment\SP weight\SP wtrt.\SP the\SP bone\ENDNOTE \NL{8}
\SP \TAB \KW{float}\SP \ID{w}\SP =\SP \ID{weight}(\ID{bone},\SP \ID{pointOnSurface});\NL{9}
\SP \TAB \NOTE transform\ENDNOTE \NL{10}
\SP \TAB \ID{vec3}\SP \ID{transformed}\SP =\SP \ID{pointOnSurface};\NL{11}
\SP \TAB \NOTE Back\SP to\SP object\SP space\ENDNOTE \NL{12}
\SP \TAB \ID{transformed}\SP +=\SP \ID{boneInvTrans}[\ID{bone}][3].\ID{xyz};\NL{13}
\SP \TAB \ID{transformed}\SP =\SP \ID{boneInvTrans}[\ID{bone}]*\ID{transformed};\NL{14}
\SP \TAB \NOTE to\SP rest\SP world\SP space\ENDNOTE \NL{15}
\SP \TAB \ID{transformed}\SP =\SP \ID{boneRestTrans}[\ID{bone}]*\ID{transformed};\NL{16}
\SP \TAB \ID{transformed}.\ID{xyz}\SP +=\SP \ID{boneRestTrans}[\ID{bone}][3].\ID{xyz};\NL{17}
\SP \TAB \NOTE sum\ENDNOTE \NL{18}
\SP \TAB \ID{restPosition}\SP +=\SP \ID{w}*\ID{transformed}.\ID{xyz};;\NL{19}
\}\NL{20}
\EMPTYLINE{21}\NOTE Compute\SP the\SP texture\SP coordinates\SP of\SP the\SP fragment\SP in\SP rest\SP position\ENDNOTE \NL{22}
\ID{vec2}\SP \ID{uv}\SP =\SP \ID{texCoord}(\ID{restPosition});\NL{23}
\EMPTYLINE{24}\end{@CPP}%
}%
