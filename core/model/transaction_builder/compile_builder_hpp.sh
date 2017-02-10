#!/bin/bash
cp builder_template __temp_builder__ \
&& (  \
  cat __temp_builder__ \
  | sed -- "s/__CommandType__/$3/g" \
  | sed -- "s/__ObjectType__/$4/g" \
  > $1/$1_$2_builder_template.hpp) && rm __temp_builder__ && ( \
  echo "/*"; cat $IROHA_HOME/LICENSE; echo "*/";
  echo "#include \"../../commands/$1.hpp\"";
  echo "#include \"../../objects/$2.hpp\"";
  echo "";
  g++ -E $1/$1_$2_builder_template.hpp \
    | clang-format -style=Google \
    | sed -e '1,7d'
) > $1/$1_$2_builder.hpp \
  && rm $1/$1_$2_builder_template.hpp