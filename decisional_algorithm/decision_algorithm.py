"""
This function is used to determine the type of garbage represented in the image. It receives as input the labels returned by
Rekognition and returns as output the name of the category assigned to the object.
"""

def classify_object(labels):
    
    #a static list of labels is created 'a priori' for every category of trash, using the most common labels obtained
    #for a series of sample images.
    organic_labels = ['Food','Salad','Ham','Meal','Lunch','Flora','Plant','Produce','Vegetable','Dessert','Burger',
                      'Fruit','Bone','Pizza','Pasta','Banana','Bread','Fish','Meat', 'Organic']
    paper_labels = ['Paper','Cardboard','Litter','Newspaper','Magazine','Text','Book','Milk', 'Business Card']
    glass_labels = ['Glass','Bottle','Beer','Window','Jar', 'Alcohol', 'Beer Glass']
    plastic_labels = ['Plastic','Dish','Plate','Cup','Fork','Bag','Bowl','Confectionery', 'Beverage', 'Bottle', 'Drink', 'Mineral Water', 'Water Bottle']
    inorganic_labels = ['Trash', 'Computer', 'Electronics', 'Laptop', 'Pc', 'Keyboard']
    
    #a counter is inizialized for every possible category of trash
    organic_count = 0
    paper_count = 0
    glass_count = 0
    plastic_count = 0
    inorganic_count = 0
    
    #count how many labels, among the ones returned by Rekognition, appear also in some of the 'a priori' lists 
    #the corresponding counter is incremented accordingly
    for l in labels:
        if l['Name'] in inorganic_labels:
            inorganic_count += 1
        if l['Name'] in organic_labels:
            organic_count += 1
        if l['Name'] in paper_labels:
            paper_count += 1
        if l['Name'] in glass_labels:
            glass_count += 1
        if l['Name'] in plastic_labels:
            plastic_count += 1
    
    #in case of equality between the counters, a generic class of Inorganic is returned 
    if organic_count == paper_count and paper_count == glass_count and glass_count == plastic_count:
        return 'Inorganic'
    
    #otherwise, the assigned class is the one corresponding to the counter with the highest value
    max_count = max([organic_count, paper_count, glass_count, plastic_count])
    if inorganic_count == max_count:
        return 'Inorganic'
    elif organic_count == max_count:
        return 'Organic'
    elif paper_count == max_count:
        return 'Paper'
    elif glass_count == max_count:
        return 'Glass'
    elif plastic_count == max_count:
        return 'Plastic'
                
                      
                     
