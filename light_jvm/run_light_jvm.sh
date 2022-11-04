timestamp=$(date +"%m_%d_%H-%M-%S")
echo $timestamp
java -XX:-UseCompressedOops -XX:-UseCompressedClassPointers -XX:+UnlockExperimentalVMOptions -XX:+UseShenandoahGC -Xlog:gc*,gc+ref*,gc+ergo*,gc+heap*,gc+stats*,gc+compaction*,gc+age*:file=gc_rdma_log/gc_${timestamp}.log -XX:+isMemServer -XX:RDMALocalAddr=192.168.0.21 -XX:RDMAPort=7471 LightJVM.java
