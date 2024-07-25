#!/bin/sh

# 定义命名管道的路径
fifo_path="/dev/test/cmd_fifo"

# 使用ipcs和grep获取特定的共享内存段ID
id=$(ipcs -m | grep "4d2" | awk '{print $2}')

# 检查id是否不为空
if [ ! -z "$id" ]; then
    # 如果共享内存段存在，则删除它
    ipcrm -m "$id"
fi

# 创建命名管道
file=$fifo_path
rm -rf $file  # 确保删除旧的命名管道文件
mkfifo $file


