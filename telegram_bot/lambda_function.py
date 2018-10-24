import telepot
import base64
import boto3
import json
import os

import requests


TOKEN = os.environ['TOKEN']
THING_NAME = os.environ['THING_NAME']
BOT_API = os.environ['BOT_API']

bot = telepot.Bot(TOKEN)
iot_client = boto3.client('iot-data')


def lambda_handler(event, context):
    msg = event['message']

    bot.sendChatAction(msg['from']['id'], action='typing')

    content_type, chat_type, chat_id = telepot.glance(msg)

    if content_type == 'photo':
        file_name = msg['photo'][-1]['file_id']
        bot.download_file(file_name, '/tmp/{}.jpg'.format(file_name))
        
        with open('/tmp/{}.jpg'.format(file_name), "rb") as image_file:
            encoded_string = base64.b64encode(image_file.read())

        # aggiorno la shadow a PROCESSING
        shadow = iot_client.update_thing_shadow(
            thingName=THING_NAME,
            payload=json.dumps({
                'state':{
                    'reported':{
                        'action': 'PROCESSING'
                    }
                }
            })
        )

        # invoco l'API
        response = requests.post(BOT_API, json={'message': encoded_string, 'type': 'Paper'}).json()
        
        # in base al risultato aggiorno lo shadow
        shadow = iot_client.update_thing_shadow(
            thingName=THING_NAME,
            payload=json.dumps({
                'state':{
                    'reported':{
                        'action': 'ACCEPTED' if response['result'] else 'REJECTED'
                    }
                }
            })
        )
        
        # restituisco l'esito
        bot.sendMessage(msg['from']['id'], 'Trash can opens!' if response['result'] else 'Wrong trash can, this is {}'.format(response['decision']))
        
        # reset dello shadow
        shadow = iot_client.update_thing_shadow(
            thingName=THING_NAME,
            payload=json.dumps({
                'state':{
                    'reported':{
                        'action': 'WAITING'
                    }
                }
            })
        )
        
        return {
            'responseCode': 200,
            'body': 'Success'
        }

    else:
        bot.sendMessage(msg['from']['id'], 'Need a picture to proceed!')
        return {
            'responseCode': 415,
            'body': 'Not supported entity'
        }