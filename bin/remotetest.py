#json_msg='{"op": "action", "exectype": 7, "scene": null, "entity": "receive_object_script", "action": "ReceiveObject", "params": ["local://fish.mesh", "local://fish.material"]}'

#json_msg='{entity.Exec(2, words[0], words[1]);}'
#json_msg='{entity.Exec(2, "Move(forward)", 1);}'

json_msg='{"op": "action", "exectype": 1, "scene": null, "entity": "FreeLookCamera", "action": "PitchAndRoll", "params": [60,0]}'


from socket import *
s = socket(AF_INET, SOCK_STREAM)

s.connect(('localhost', 4242))
s.sendall(json_msg + '\r\n')
#print json_msg
print s.recv(1024)
s.close()
