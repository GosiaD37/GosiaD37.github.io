import os
from jupyter_dash import JupyterDash
from dash import dcc, html, Input, Output
from dash.dash_table import DataTable
import dash_leaflet as dl
import pandas as pd
from pymongo import MongoClient
import plotly.express as px
import base64

# ============================
# MongoDB connection settings
# ============================
MONGO_HOST = "nv-desktop-services.apporto.com"
MONGO_PORT = "31988"
MONGO_USER = "aacuser"
MONGO_PASS = "Nikispring7"
MONGO_DB = "AAC"
MONGO_COLLECTION = "animals"

# Connection string construction
connection_string = f"mongodb://{MONGO_USER}:{MONGO_PASS}@{MONGO_HOST}:{MONGO_PORT}/{MONGO_DB}?authSource=admin"

# Establish MongoDB connection
client = MongoClient(connection_string)
db = client[MONGO_DB]
collection = db[MONGO_COLLECTION]

# ============================
# Dash App Initialization
# ============================
app = JupyterDash(__name__)

# Load and encode Grazioso Salvare logo
LOGO_PATH = "/home/malgorzatadeb_snhu/Desktop/Grazioso Salvare Logo.png"
logo_encoded = base64.b64encode(open(LOGO_PATH, "rb").read())

# ============================
# Query Functions (Modularized)
# ============================

def query_water_rescue():
    return {
        '$and': [
            {'sex_upon_outcome': 'Intact Female'},
            {'breed': {'$in': ['Labrador Retriever Mix', 'Chesapeake Bay Retriever', 'Newfoundland']}},
            {'age_upon_outcome_in_weeks': {'$gte': 26, '$lte': 156}}
        ]
    }

def query_mountain_rescue():
    return {
        '$and': [
            {'sex_upon_outcome': 'Intact Male'},
            {'breed': {'$in': ['German Shepherd', 'Alaskan Malamute', 'Old English Sheepdog', 'Siberian Husky', 'Rottweiler']}},
            {'age_upon_outcome_in_weeks': {'$gte': 26, '$lte': 156}}
        ]
    }

def query_disaster_tracking():
    return {
        '$and': [
            {'sex_upon_outcome': 'Intact Male'},
            {'breed': {'$in': ['Doberman Pinscher', 'German Shepherd', 'Golden Retriever', 'Bloodhound', 'Rottweiler']}},
            {'age_upon_outcome_in_weeks': {'$gte': 20, '$lte': 300}}
        ]
    }

# ============================
# Efficient Query Dispatcher
# ============================

filter_map = {
    'WR': query_water_rescue,
    'MWR': query_mountain_rescue,
    'DRIT': query_disaster_tracking,
    'RESET': lambda: {
        '$or': [
            query_water_rescue(),
            query_mountain_rescue(),
            query_disaster_tracking()
        ]
    }
}

# Lookup query efficiently using dictionary
def build_query(filter_type):
    return filter_map.get(filter_type, lambda: {})()

# ============================
# Reusable Data Preprocessing
# ============================

def preprocess_dataframe(data):
    df = pd.DataFrame(data)
    if '_id' in df.columns:
        df.drop(columns=['_id'], inplace=True)
    if 'breed' in df.columns:
        df['breed'] = df['breed'].str.strip().str.title()
    return df

# ============================
# App Layout
# ============================

app.layout = html.Div([
    # Header and Logo
    html.Div([
        html.A(html.Img(src=f"data:image/png;base64,{logo_encoded.decode()}",
                        style={'width': '200px'}), href="https://www.snhu.edu"),
    ], style={'textAlign': 'center'}),
    html.H1("Grazioso Salvare Dashboard - Malgorzata Debska", style={"textAlign": "center", "color": "blue"}),

    # Filtering Options
    html.Div([
        dcc.RadioItems(
            id='filter-type',
            options=[
                {'label': 'Water Rescue', 'value': 'WR'},
                {'label': 'Mountain/Wilderness Rescue', 'value': 'MWR'},
                {'label': 'Disaster/Individual Tracking', 'value': 'DRIT'},
                {'label': 'Reset', 'value': 'RESET'}
            ],
            value='RESET',
            labelStyle={'display': 'block'}
        )
    ], style={'padding': '10px', 'maxWidth': '400px', 'margin': '0 auto'}),

    # Data Table
    DataTable(
        id='datatable-id',
        columns=[],
        style_table={'overflowX': 'auto'},
        page_current=0,
        page_size=10,
        page_action='custom',
        row_selectable='multi'
    ),

    # Charts and Map
    html.Div([
        html.Div(id='pie-chart', className='six columns'),
        html.Div(id='map-id', className='six columns')
    ], className='row', style={'display': 'flex', 'justifyContent': 'space-around', 'marginTop': '20px'}),

    html.Footer("Dashboard created by Malgorzata Debska", style={"textAlign": "center", "color": "gray", "marginTop": "20px"})
])

# ============================
# Callback: Update Table
# ============================

@app.callback(
    Output('datatable-id', 'columns'),
    Output('datatable-id', 'data'),
    [Input('filter-type', 'value')]
)
def update_table(filter_type):
    query = build_query(filter_type)
    data = list(collection.find(query))
    if not data:
        return [], []
    df = preprocess_dataframe(data)

    # Remove duplicates based on specific identifying columns
    df = df.drop_duplicates(subset=['name', 'breed', 'age_upon_outcome_in_weeks'])

    columns = [{"name": col, "id": col} for col in df.columns]
    return columns, df.to_dict('records')

# ============================
# Callback: Update Pie Chart
# ============================

@app.callback(
    Output('pie-chart', 'children'),
    [Input('filter-type', 'value')]
)
def update_pie_chart(filter_type):
    query = build_query(filter_type)
    data = list(collection.find(query))
    if not data:
        return "No data to display."
    df = preprocess_dataframe(data)

    title_map = {
        'WR': 'Water Rescue',
        'MWR': 'Mountain/Wilderness Rescue',
        'DRIT': 'Disaster/Individual Tracking',
        'RESET': 'Dog Breed Distribution'
    }
    title = title_map.get(filter_type, 'Dog Breed Distribution')

    fig = px.pie(df, names='breed', title=title)
    fig.update_layout(
        legend=dict(
            orientation="h",
            yanchor="top",
            y=-0.3,
            xanchor="center",
            x=0.5,
            font=dict(size=10)
        ),
        title=dict(x=0.5)
    )
    return dcc.Graph(figure=fig, style={'height': '400px', 'width': '400px', 'margin': 'auto'})

# ============================
# Callback: Update Map
# ============================

@app.callback(
    Output('map-id', 'children'),
    [Input('datatable-id', 'data'),
     Input('datatable-id', 'selected_rows')]
)
def update_map(view_data, selected_rows):
    if not view_data:
        return "No data to display on map."
    df = pd.DataFrame(view_data)

    # Ensure necessary columns exist
    if 'location_lat' not in df.columns or 'location_long' not in df.columns:
        return "Location data not available for display."
    if not selected_rows:
        return "No rows selected for map display."

    markers = []
    for row_idx in selected_rows:
        if row_idx < len(df):
            lat = df.iloc[row_idx]['location_lat']
            long = df.iloc[row_idx]['location_long']
            markers.append(dl.Marker(position=[lat, long], children=[
                dl.Tooltip(df.iloc[row_idx]['breed']),
                dl.Popup([
                    html.H4("Animal Name"),
                    html.P(df.iloc[row_idx]['name']),
                    html.H4("Breed"),
                    html.P(df.iloc[row_idx]['breed']),
                ])
            ]))

    if markers:
        lat = df.iloc[selected_rows[0]]['location_lat']
        long = df.iloc[selected_rows[0]]['location_long']
        return dl.Map(style={'width': '600px', 'height': '400px'}, center=[lat, long], zoom=10, children=[
            dl.TileLayer(id="base-layer-id"),
            *markers
        ])
    else:
        return "No valid rows selected for map display."

# ============================
# Run App
# ============================

app.run_server(mode='inline', port=8044)
