#pragma once

bool async_publish(char *server, char *client, char *topic, char *payload, char * lastwill, int qos);
