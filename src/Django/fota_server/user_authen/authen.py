import pyodbc
from pyodbc import Error



def sql_authen_api(username,password):
    try:
        connectionQuery = "DRIVER={{SQL Server}}; SERVER=XXX,1433; Database=TestDatabase; UID={}; PWD={};Trusted_Connection=no".format(username,password)
        conn = pyodbc.connect(connectionQuery)
        return conn.cursor()
    except Error as e:
        print(e)