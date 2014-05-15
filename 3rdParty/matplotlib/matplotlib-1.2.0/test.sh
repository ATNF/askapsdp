#!/usr/bin/env sh
. ./init_package_env.sh

python << EOF
import matplotlib
print matplotlib.get_backend()
import matplotlib.pyplot as plt
plt.plot(range(10), range(10))
plt.title("Simple Plot")
plt.show()
EOF
