#!/bin/sh

## generates a wrapper object for a special (openGL) function

type2pd () {
    echo "t_float"
}

type2pdarg () {
    echo "t_floatarg"
}

type2PD () {
    echo "A_DEFFLOAT"
}

make_header () {
echo "/* ------------------------------------------------------------------"
echo " * GEM - Graphics Environment for Multimedia"
echo " *"
echo " *  Copyright (c) 2008 zmoelnig@iem.at"
echo " *  For information on usage and redistribution, and for a DISCLAIMER"
echo " *  OF ALL WARRANTIES, see the file, \"GEM.LICENSE.TERMS\""
echo " *"
echo " *  this file has been generated..."
echo " * ------------------------------------------------------------------"
echo " */"
echo ""
echo "#ifndef INCLUDE_GEM_${FUN_NAME}_H_"
echo "#define INCLUDE_GEM_${FUN_NAME}_H_"
echo ""
echo "#include \"GemGLBase.h\""
echo ""
echo "/*"
echo " CLASS"
echo "	${gem_name}"
echo " KEYWORDS"
echo "	openGL"
echo " DESCRIPTION"
echo "	wrapper for the openGL-function"
echo "	\"${line}\""
echo " */"
echo ""
echo "class GEM_EXTERN ${gem_name} : public GEMglBase"
echo "{"
echo "	CPPEXTERN_HEADER(${gem_name}, GEMglBase)"
echo ""
echo "	public:"
echo "	  // Constructor"
echo -n "	  ${gem_name} ("

i=0
while [ ${i} -lt ${fun_argcount} ]
do
  echo -n "$(type2pd ${fun_argtypes[${i}]})"
  i=$((i+1))
  if [ ${i} -lt  ${fun_argcount} ]
  then
    echo -n ", "
  fi
# real-types to pd-types
done
echo ");"
echo "";

echo "	protected:"
echo "	  // Destructor"
echo "	  virtual ~${gem_name} ();"
echo "	  // Do the rendering"
echo "	  virtual void	render (GemState *state);"
echo ""
echo "    // variables"

i=0
while [ ${i} -lt ${fun_argcount} ]
do
  echo "	  ${fun_argtypes[${i}]} m_${fun_argnames[${i}]}; // VAR"
  echo "	  virtual void ${fun_argnames[${i}]}Mess( $(type2pd ${fun_argtypes[${i}]}) ); // VAR"

  echo ""
  i=$((i+1))
done
echo ""
echo "	private:"
echo ""
echo "    // we need some inlets"
echo "	  t_inlet *m_inlet[${fun_argcount}];"
echo ""
echo "    // static member functions"

i=0
while [ ${i} -lt ${fun_argcount} ]
do
  echo "	  static void ${fun_argnames[${i}]}MessCallback(void*, $(type2pdarg ${fun_argtypes[${i}]}) );"
  i=$((i+1))
done

echo "};"
echo "#endif /* for header file */"
}

make_body () {
echo "////////////////////////////////////////////////////////"
echo "//"
echo "// GEM - Graphics Environment for Multimedia"
echo "//"
echo "// Implementation file"
echo "//"
echo "// Copyright (c) 2008 zmoelnig@iem.at"
echo "//  For information on usage and redistribution, and for a DISCLAIMER"
echo "//  OF ALL WARRANTIES, see the file \"GEM.LICENSE.TERMS\""
echo "//"
echo ""
echo "#include \"${header_file}\""
echo ""
# CPPEXTERN...
echo "/////////////////////////////////////////////////////////"
echo "//"
echo "// ${gem_name}"
echo "//"
echo "/////////////////////////////////////////////////////////"
echo "// Constructor"
echo "//"
echo -n "${gem_name} :: ${gem_name}	("
i=0
while [ ${i} -lt ${fun_argcount} ]
do
  echo -n "$(type2pdarg ${fun_argtypes[${i}]}) arg${i}=0"
  i=$((i+1))
  if [ ${i} -lt  ${fun_argcount} ]
  then
    echo -n ", "
  fi
  echo ""
done
echo ") :"
i=0
while [ ${i} -lt ${fun_argcount} ]
do
  echo -n "	  m_${fun_argnames[${i}]}((${fun_argtypes[${i}]})arg${i})"
  i=$((i+1))
  if [ ${i} -lt  ${fun_argcount} ]
  then
    echo -n ","
  fi
  echo ""
done
echo "{"
i=0
while [ ${i} -lt ${fun_argcount} ]
do
  echo "	  m_inlet[${i}] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym(\"${fun_argnames[${i}]}\"));"
  i=$((i+1))
done
echo "}"
echo ""


echo "/////////////////////////////////////////////////////////"
echo "// Destructor"
echo "//"
echo "${gem_name} :: ~${gem_name} () {"
i=0
while [ ${i} -lt ${fun_argcount} ]
do
  echo "	  inlet_free(m_inlet[${i}]); m_inlet[${i}]=NULL;"

  i=$((i+1))
done
echo "}"

echo ""
echo "/////////////////////////////////////////////////////////"
echo "// Render"
echo "//"
echo "void ${gem_name} :: render(GemState *state) {"
echo -n "	${fun_name} ("
i=0
while [ ${i} -lt ${fun_argcount} ]
do
  echo -n "m_${fun_argnames[${i}]}"
  i=$((i+1))
  if [ ${i} -lt  ${fun_argcount} ]
  then
    echo -n ", "
  fi
done
echo ");"
echo "}"
echo ""

i=0
while [ ${i} -lt ${fun_argcount} ]
do
  echo "void ${gem_name} :: ${fun_argnames[${i}]}Mess($(type2pd ${fun_argtypes[${i}]}) arg1) {"
  echo "	  m_${fun_argnames[${i}]}=(${fun_argtypes[${i}]})arg1;"
  echo "	  setModified();"
  echo "}"
  echo ""
  i=$((i+1))
done


echo ""
echo "/////////////////////////////////////////////////////////"
echo "// static member functions"
echo "//"
echo ""
echo "void ${gem_name} :: obj_setupCallback(t_class *classPtr) {"
i=0
while [ ${i} -lt ${fun_argcount} ]
do
  echo "	 class_addmethod(classPtr, (t_method)&${gem_name}::${fun_argnames[${i}]}MessCallback,  	gensym(\"${fun_argnames[${i}]}\"), $(type2PD ${fun_argtypes[${i}]}), A_NULL);"
  i=$((i+1))
done
echo "};"
echo ""
i=0
while [ ${i} -lt ${fun_argcount} ]
do
  echo "void ${gem_name} :: ${fun_argnames[${i}]}MessCallback (void*data, $(type2pdarg ${fun_argtypes[${i}]}) arg0) {"
  echo "	GetMyClass(data)->${fun_argnames[${i}]}Mess( ($(type2pd ${fun_argtypes[${i}]})) arg0);"
  echo "}"
  i=$((i+1))
done


echo ""

}



parsedecl () {
    local fun_return
    local fun_name
# array of argument names
    local fun_argnames
# array of argument types
    local fun_argtypes
#number of arguments
    local fun_argcount
    local dummy
    local line

    local gem_name

    line=$@

    fun_return=$1
    fun_name=$2
    shift 2

    fun_argcount=0
    while [ $# -gt 0 ]
    do
      dummy=$1
      dummy=${dummy#(}
      dummy=${dummy%,}
      dummy=${dummy%)}
      dummy=${dummy%;}
      fun_argtypes[${fun_argcount}]="${dummy}"

      dummy=$2
      dummy=${dummy#(}
      dummy=${dummy%,}
      dummy=${dummy%;}
      dummy=${dummy%)}

      fun_argnames[${fun_argcount}]="${dummy}"
      shift 2
      fun_argcount=$((fun_argcount+1))
    done

    gem_name="GEM${fun_name}"
    FUN_NAME=$(echo ${fun_name} | awk '{print toupper($1)}')


    header_file="${gem_name}.h"
    body_file="${gem_name}.cpp"

    if [ -e "${header_file}" ]; then
        exit 1
    fi
    if [ -e "${body_file}" ]; then
        exit 1
    fi

    make_header > ${header_file}
    make_body > ${body_file}
}



# usage: echo "void gluPerspective (GLdouble fovy, GLdouble aspect, GLdouble zNear, GLdouble zFar);" | ./fun2object.sh
while read line
do
  parsedecl ${line}
done
