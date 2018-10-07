"""
This script is used to handle the reception of the image acquired by the ArduCAM and sent through MQTT. This image is converted
to jpeg, uploaded to S3 and analysed by means of AWS Rekognition. The function defined in 'decision_algorithm.py' is finally invoked
in order to determine the type of garbage.
"""

import boto3, base64, decision_algorithm, datetime, json, time, os
from decimal import *

# s3 bucket
BUCKET_NAME = os.environ['BUCKET_NAME']

# dynamo table
TABLE = os.environ['TABLE']

# iot topic for responses
RESPONSE_TOPIC = os.environ['RESPONSE_TOPIC']

#aws resources
s3 = boto3.resource('s3')
dynamodb = boto3.resource('dynamodb')
rekognition = boto3.client('rekognition')
iot = boto3.client('iot-data')

def lambda_handler(event, context):
    now = int(time.time())
    file_name = str(now) + '.jpg'      #the name of the image is set according to the date and time of reception
    encoded_image = event['message']   #the image is received as an MQTT message 

    #convert base64 string to jpg
    temp_file = '/tmp/{}'.format(file_name)   #the image is sent as a base64 string; it needs to be converted to jpeg in order to
                                              #be handled by Rekognition

    try:
        with open(temp_file, 'wb') as image_writer:
            image_writer.write(base64.b64decode(encoded_image))
            
        #upload jpg file to s3
        s3.Bucket(BUCKET_NAME).upload_file(temp_file, file_name, ExtraArgs={'ACL':'public-read'})
        
        #invoke Rekognition to get a list of labels for the image
        response = rekognition.detect_labels(Image={'S3Object':{'Bucket':BUCKET_NAME,'Name': file_name}})
        
        print('Detected labels for {}'.format(file_name))    
        for label in response['Labels']:
            print (label['Name'] + ' : ' + str(label['Confidence']))
        
        #call the decison algorithm function to determine the type of garbage
        decision = decision_algorithm.classify_object(response['Labels'])
        
        # create a table object
        table = dynamodb.Table(TABLE)
        
        #for every evaluation, we save the id (determined by the time), the filename, the labels returned by Rekognition and the class assigned
        #by the decision algorithm.
        timestamp = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        labels = [l['Name'] for l in response['Labels']]
        item = {'item_id' : now,
                'file_name' : file_name,
                'labels': labels,
                'assigned_class' : decision,
        }
        
        #save record to db
        table.put_item(Item=item)
        
        #publish the result of the evaluation through MQTT
        iot.publish(topic=RESPONSE_TOPIC, payload= json.dumps({'assigned_class' : decision}))
        
    except:
        decision = event['name']
        result = '+' if decision == event['type'] else '-'
        iot.publish(topic=RESPONSE_TOPIC, payload= result)
    
    return {
        'decision': decision,
        'result': decision == event['type'],
        'labels': labels
    }