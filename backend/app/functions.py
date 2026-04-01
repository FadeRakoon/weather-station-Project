 #!/usr/bin/python3


#################################################################################################################################################
#                                                    CLASSES CONTAINING ALL THE APP FUNCTIONS                                                                                                    #
#################################################################################################################################################


class DB:

    def __init__(self,Config):

        from math import floor
        from os import getcwd
        from os.path import join
        from json import loads, dumps, dump
        from datetime import timedelta, datetime, timezone 
        from pymongo import MongoClient , errors, ReturnDocument
        from urllib import parse
        from urllib.request import  urlopen 
        from bson.objectid import ObjectId  
       
      
        self.Config                         = Config
        self.getcwd                         = getcwd
        self.join                           = join 
        self.floor                      	= floor 
        self.loads                      	= loads
        self.dumps                      	= dumps
        self.dump                       	= dump  
        self.datetime                       = datetime
        self.ObjectId                       = ObjectId 
        self.server			                = Config.DB_SERVER
        self.port			                = Config.DB_PORT
        self.username                   	= parse.quote_plus(Config.DB_USERNAME)
        self.password                   	= parse.quote_plus(Config.DB_PASSWORD)
        self.remoteMongo                	= MongoClient
        self.ReturnDocument                 = ReturnDocument
        self.PyMongoError               	= errors.PyMongoError
        self.BulkWriteError             	= errors.BulkWriteError  
        self.tls                            = False # MUST SET TO TRUE IN PRODUCTION


    def __del__(self):
            # Delete class instance to free resources
            pass
 


    # ####################
    # # WEATHER STATION DATABASE FUNCTIONS  #
    # ####################
    
    def addUpdate(self, data):
        '''Add a new sensor reading to the Station collection'''
        try:
            remotedb = self.remoteMongo('mongodb://%s:%s@%s:%s' % (self.username, self.password, self.server, self.port), tls=self.tls)
            result = remotedb.ELET2415.Station.insert_one(data)
        except Exception as e:
            msg = str(e)
            if "duplicate" not in msg:
                print("addUpdate error ", msg)
            return False
        else:                  
            return True
        
       

    def getAllInRange(self, start, end, station_id=None):
        '''Returns all sensor data within the specified timestamp range, optionally filtered by station_id'''
        try:
            remotedb = self.remoteMongo('mongodb://%s:%s@%s:%s' % (self.username, self.password, self.server, self.port), tls=self.tls)
            query = {"timestamp": {"$gte": int(start), "$lte": int(end)}}
            if station_id:
                query["station_id"] = station_id
            result = list(remotedb.ELET2415.Station.find(query, {"_id": 0}).sort("timestamp", 1))
        except Exception as e:
            msg = str(e)
            print("getAllInRange error ", msg)
        else:
            return result
        

    def humidityMMAR(self, start, end, station_id=None):
        '''Returns min, max, avg, and range for humidity within timestamp range, optionally filtered by station_id'''
        try:
            remotedb = self.remoteMongo('mongodb://%s:%s@%s:%s' % (self.username, self.password, self.server, self.port), tls=self.tls)
            query = {"timestamp": {"$gte": int(start), "$lte": int(end)}}
            if station_id:
                query["station_id"] = station_id
            result = list(remotedb.ELET2415.Station.aggregate([{"$match": query}, {"$group": {"_id": None, "humidity": {"$push": "$$ROOT.humidity"}}}, {"$project": {"_id": 0, "max": {"$max": "$humidity"}, "min": {"$min": "$humidity"},"avg": {"$avg": "$humidity"}, "range": {"$subtract": [{"$max": "$humidity"}, {"$min": "$humidity"}]}}}]))
        except Exception as e:
            msg = str(e)
            print("humidityMMAR error ", msg)
        else:
            return result
    
    def temperatureMMAR(self, start, end, station_id=None):
        '''Returns min, max, avg, and range for temperature within timestamp range, optionally filtered by station_id'''
        try:
            remotedb = self.remoteMongo('mongodb://%s:%s@%s:%s' % (self.username, self.password, self.server, self.port), tls=self.tls)
            query = {"timestamp": {"$gte": int(start), "$lte": int(end)}}
            if station_id:
                query["station_id"] = station_id
            result = list(remotedb.ELET2415.Station.aggregate([{"$match": query}, {"$group": {"_id": None, "temperature": {"$push": "$$ROOT.temperature"}}}, {"$project": {"_id": 0, "max": {"$max": "$temperature"}, "min": {"$min": "$temperature"},"avg": {"$avg": "$temperature"}, "range": {"$subtract": [{"$max": "$temperature"}, {"$min": "$temperature"}]}}}]))
        except Exception as e:
            msg = str(e)
            print("temperatureMMAR error ", msg)
        else:
            return result

    def soilMoistureMMAR(self, start, end, station_id=None):
        '''Returns min, max, avg, and range for soil moisture within timestamp range, optionally filtered by station_id'''
        try:
            remotedb = self.remoteMongo('mongodb://%s:%s@%s:%s' % (self.username, self.password, self.server, self.port), tls=self.tls)
            query = {"timestamp": {"$gte": int(start), "$lte": int(end)}}
            if station_id:
                query["station_id"] = station_id
            result = list(remotedb.ELET2415.Station.aggregate([{"$match": query}, {"$group": {"_id": None, "soil_moisture": {"$push": "$$ROOT.soil_moisture"}}}, {"$project": {"_id": 0, "max": {"$max": "$soil_moisture"}, "min": {"$min": "$soil_moisture"},"avg": {"$avg": "$soil_moisture"}, "range": {"$subtract": [{"$max": "$soil_moisture"}, {"$min": "$soil_moisture"}]}}}]))
        except Exception as e:
            msg = str(e)
            print("soilMoistureMMAR error ", msg)
        else:
            return result

    def getDistinctStations(self):
        '''Returns list of unique station_ids from database'''
        try:
            remotedb = self.remoteMongo('mongodb://%s:%s@%s:%s' % (self.username, self.password, self.server, self.port), tls=self.tls)
            result = remotedb.ELET2415.Station.distinct("station_id")
        except Exception as e:
            msg = str(e)
            print("getDistinctStations error ", msg)
        else:
            return result


    def frequencyDistro(self, variable, start, end):
        '''Returns frequency distribution for a variable within timestamp range'''
        try:
            remotedb = self.remoteMongo('mongodb://%s:%s@%s:%s' % (self.username, self.password, self.server, self.port), tls=self.tls)
            result = list(remotedb.ELET2415.Station.aggregate([{"$match": {"timestamp": {"$gte": int(start), "$lte": int(end)}}}, {"$bucket": {"groupBy": "$" + variable, "boundaries": [0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100], "default": "outliers", "output": {"count": {"$sum": 1}}}}]))
        except Exception as e:
            msg = str(e)
            print("frequencyDistro error ", msg)            
        else:                  
            return result
        
 



def main():
    from config import Config
    from time import time, ctime, sleep
    from math import floor
    from datetime import datetime, timedelta
    one = DB(Config)
 
 
    start = time() 
    end = time()
    print(f"completed in: {end - start} seconds")
    
if __name__ == '__main__':
    main()


    
