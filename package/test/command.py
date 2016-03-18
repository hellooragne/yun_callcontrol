#!/usr/bin/env python

import string
import sys
import os
import time

class command():

    def __init__(self):
        pass

    def __del__(self):
        pass

    def get_s_add(self, agent):
        s = '"dmcc_config add ' + agent + ' 127.0.0.1 3000"'
        return s

    def get_s_del(self, agent):
		s = '"dmcc_config del ' + agent + ' "'
		return s

    def first(self, pwd):
		i = 0
		s_command = 'python ' + pwd + 'single_command.py' + ' -c '
		while i <= 500000:
			j = 0
			k = 0
			while j <= 100:
				s = self.get_s_add(str(j + 2000))
				os.system(s_command + s)
				j += 1
			while k <= 100:
				s = self.get_s_del(str(k + 2000))
				os.system(s_command + s)
				k += 1
			time.sleep(0.1)
			i += 1
			pass



if __name__ == "__main__":
    testing_tmp = command()
    testing_tmp.first('/home/ctrip/software/csta/freeswitch-1.4.0/libs/esl/python/')
