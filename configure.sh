num_core=`nproc --all`

./configure --enable-debug --with-jvm-features=shenandoahgc --with-num-cores=${num_core} --with-jobs=${num_core}