// Copyright 2013 Alexandre Fiori
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Event Socket client that connects to FreeSWITCH to originate a new call.
package main

import (
	"encoding/json"
	"fmt"
	"github.com/fiorix/go-eventsocket/eventsocket"
	"log"
)

type Profile struct {
	Name  string
	Value string
}

const originator_uuid = "2242dee0-35b6-11e9-b900-355947430f71"

//const dialplan = "&socket(localhost:9090 async)"

func main() {
	client, err := eventsocket.Dial("localhost:8021", "ClueCon")
	if err != nil {
		log.Fatal(err)
	}
	client.Send("events json CHANNEL_CREATE CHANNEL_DESTROY CHANNEL_ANSWER CHANNEL_HANGUP CHANNEL_CALLSTATE CUSTOM robot::report")
	//c.Send("events json ALL")

	var profile = []Profile{{"Alex", "snowboarding"}, {"Six", "swimming"}}
	js, err := json.Marshal(profile)
	if err != nil {
		return
	}

	//c.Send(fmt.Sprintf("bgapi originate %s %s", dest, dialplan))

	fmt.Printf("Testing JSON %s\n", js)

	jsondata := []byte(`{"version":"1.0"\,"sequence":"6757"\,"plan_id":"10050"\,"robot_index":"10"\,"call_id":"a-3892e9971c5d4b378b4fe17e9770d4d8"\,"caller_id":"2"\,"callee":"91008"\,"asr_engine":"keda"\,"template_id":"template_001"\,"audio_record":"1"\,"record_path":"/tmp/log/7892_10050_1017_1008"\,"early_media_record":"1"\,"nlp_server":"10.200.1.37:18086"\,"asr_server":"10.20.5.44:4000"}`)

	client.Send(fmt.Sprintf("bgapi originate {origination_caller_id_number=853692}sofia/external/18625221611@140.206.80.114 &bridge({origination_uuid=2242dee0-35b6-11e9-b900-355947430f71,sip_status=183,robot_input=%s}Robot)\n", jsondata))

	for {
		ev, err := client.ReadEvent()
		if err != nil {
			log.Fatal(err)
		}

		fmt.Println("Received Event", ev.Get("Event-Name"))

		if ev.Get("Event-Name") == "CHANNEL_ANSWER" {
			fmt.Println("UUID", ev.Get("Unique-Id"))
			if ev.Get("Other-Leg-Unique-Id") == originator_uuid {
				client.Send("bgapi uuid_send_message 2242dee0-35b6-11e9-b900-355947430f71 sip_status")
			}
		}
		if ev.Get("Event-Name") == "CUSTOM" {
			ev.PrettyPrint()
		}

		/*
			if ev.Get("Answer-State") == "hangup" {
				ev.PrettyPrint()
				break
			}
		*/
	}
	client.Close()
}
