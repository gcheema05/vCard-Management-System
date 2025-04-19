#!/usr/bin/env python3
 
# Name: Gurbaaz Singh Cheema
# Date: March 27 2025

from asciimatics.widgets import Frame, ListBox, Layout, Divider, Text, \
    Button, TextBox, Widget
from asciimatics.scene import Scene
from asciimatics.screen import Screen
from asciimatics.exceptions import ResizeScreenError, NextScene, StopApplication
import sys
import sqlite3

# Import the ctypes library
from ctypes import *

# Import the mysql.connector library
import mysql.connector

# Import os library
import os

# Import the datetime library
from datetime import datetime

# Save the path to the library for future reference
parserLibPath = './libvcparser.so'

# Create a reference to the library module
vcParser = CDLL(parserLibPath)

class ContactModel():
    def __init__(self):
        # Initialize with no database connection.
        self._db = None
        self.current_id = None

    # Inserts a file entry with given file name into the FILE table, makes last_modified and creation_time = NOW() which be current time
    def create_file(self, file_name):
        cursor = self._db.cursor()
        cursor.execute('''INSERT INTO FILE (file_name, last_modified, creation_time) 
                          VALUES (%s, NOW(), NOW())''', 
                       (file_name,))
        self._db.commit()  # Commit the transaction manually

    # Inserts a file entry with given file name into the FILE table, makes last_modified the modification time given by the os
    def add_file(self, file_name):
        last_modified_time = datetime.fromtimestamp(os.path.getmtime("cards/" + file_name))
        cursor = self._db.cursor()
        cursor.execute('''INSERT INTO FILE (file_name, last_modified, creation_time) 
                          VALUES (%s, %s, NOW())''', 
                       (file_name, last_modified_time,))
        self._db.commit()  # Commit the transaction manually

    # Inserts a contact entry with given name and file_id into the CONTACT table
    def add_contact(self, name, file_id):
        cursor = self._db.cursor()
        cursor.execute('''INSERT INTO CONTACT (name, birthday, anniversary, file_id) 
                          VALUES (%s, NULL, NULL, %s)''', 
                       (name, file_id,))
        self._db.commit()  # Commit the transaction manually

    # Inserts a contact entry with given name, bday, ann, and file_id into the CONTACT table
    def create_contact(self, name, bday, ann, file_id):
        cursor = self._db.cursor()
        cursor.execute('''INSERT INTO CONTACT (name, birthday, anniversary, file_id) 
                          VALUES (%s, %s, %s, %s)''', 
                       (name, bday, ann, file_id,))
        self._db.commit()  # Commit the transaction manually

    # Updates a contact entry with given file_id to change the last_modified to NOW()
    def update_file(self, file_id):
        cursor = self._db.cursor()
        cursor.execute('''UPDATE FILE SET last_modified=NOW() WHERE file_id=%s''',
                       (file_id,))
        self._db.commit()  # Commit the transaction manually

    # Updates a contact entry with given name of given file_id
    def update_contact(self, name, file_id):
        cursor = self._db.cursor()
        cursor.execute('''UPDATE CONTACT SET name=%s WHERE file_id=%s''',
                       (name, file_id,))
        self._db.commit()  # Commit the transaction manually

    # Selects the file_name and file_id columns from the FILE table and returns as a tuple containing the result
    def get_summary(self):
        cursor = self._db.cursor()
        cursor.execute("SELECT file_name, file_id FROM FILE")
        return cursor.fetchall()

    # Selects the file_id based on given file_name and returns it
    def get_file_id(self, file_name):
        cursor = self._db.cursor()
        cursor.execute("SELECT file_id FROM FILE WHERE file_name=%s", (file_name,))
        result = cursor.fetchone()
        if result:
            return result[0]  # Return only the file_id
        return None  # No file found

    # Selects the file_name based on given file_id and returns it
    def get_file_name(self, file_id):
        cursor = self._db.cursor()
        cursor.execute("SELECT file_name FROM FILE WHERE file_id=%s", (file_id,))
        result = cursor.fetchone()
        if result:
            return result[0]  # Return only the file_name
        return None  # No file found

    # Will read files from the cards/ folder into the database
    def get_files_from_folder(self, path):
        if not os.path.isdir(path):
            return

        cursor = self._db.cursor()

        # Iterate through all files in the folder
        for file_name in os.listdir(path):
            file_path = os.path.join(path, file_name)

            # Ensure it's a file, not a folder
            if os.path.isfile(file_path):
                # Check if the file already exists in the database
                cursor.execute("SELECT file_id FROM FILE WHERE file_name=%s", (file_name,))
                result = cursor.fetchone()

                if not result:
                    full_file_name = "cards/" + file_name
                    cfileName = full_file_name.encode('utf-8')

                    # Set the return type of createValidateStr(char * fileName) to c_char_p
                    vcParser.createValidateStr.restype = c_char_p

                    # Call the createValidateStr and store the return return C string in cDetails
                    cDetails = vcParser.createValidateStr(c_char_p(cfileName))

                    # Convert the C string into a python string
                    pDetails = cDetails.decode('utf-8')

                    if not (pDetails == "NULL"):

                        # Split the string by ;
                        details_list = pDetails.split(';')

                        # Set the contact value field
                        contact = details_list[0]

                        # Set the birthday value field
                        bday = details_list[1]

                        # Set the ann value field
                        ann = details_list[2]

                        # Convert the bday and ann values into datetime values
                        if bday == "":
                            bday_datetime = None
                        else:
                            if bday.endswith('Z'):
                                bday_datetime = datetime.strptime(bday, "%Y%m%dT%H%M%SZ")
                            else:
                                bday_datetime = datetime.strptime(bday, "%Y%m%dT%H%M%S")
                        if ann == "":
                            ann_datetime = None
                        else:
                            if ann.endswith('Z'):
                                ann_datetime = datetime.strptime(ann, "%Y%m%dT%H%M%SZ")
                            else:
                                ann_datetime = datetime.strptime(ann, "%Y%m%dT%H%M%S")

                        # Insert into the database
                        self.add_file(file_name)
                        file_id = self.get_file_id(file_name)
                        self.create_contact(contact, bday_datetime, ann_datetime, file_id)

                    
        self._db.commit()  # Commit all changes

    # Selects every column expect for file_id from the CONTACT table and only column file_name for the FILE table
    def get_all_contacts(self):
        cursor = self._db.cursor()
        cursor.execute("SELECT contact_id, name, birthday, anniversary, file_name FROM FILE\
                        JOIN CONTACT ON FILE.file_id=CONTACT.file_id ORDER BY file_name")
        return cursor.fetchall()
    
    # Selects the name and birthday columns from CONTACT based on if a matching file_id and a birthday month of June is present, sort by age
    def born_in_june(self):
        cursor = self._db.cursor()
        cursor.execute("SELECT name, birthday FROM FILE\
                        JOIN CONTACT ON FILE.file_id=CONTACT.file_id AND MONTH(CONTACT.birthday)=6\
                        ORDER BY ABS(DATEDIFF(FILE.last_modified, CONTACT.birthday)/365)")
        return cursor.fetchall()

    # Will return the number of rows or files in the database
    def count_rows_file(self):
        cursor = self._db.cursor()
        cursor.execute("SELECT COUNT(*) FROM FILE")
        result = cursor.fetchone()
        if result:
            return result[0]  # Return only the number of rows
        return 0  # Empty
    
    # Will return the number of rows or contacts in the database
    def count_rows_contact(self):
        cursor = self._db.cursor()
        cursor.execute("SELECT COUNT(*) FROM CONTACT")
        result = cursor.fetchone()
        if result:
            return result[0]  # Return only the number of rows
        return 0  # Empty

class ListView(Frame):
    def __init__(self, screen, model):
        # Save off the model that accesses the contacts database.
        self._model = model

        super(ListView, self).__init__(screen,
                                       screen.height * 2 // 3,
                                       screen.width * 2 // 3,
                                       on_load=self._reload_list,
                                       hover_focus=True,
                                       can_scroll=False,
                                       title="vCard List")

        # Create the form for displaying the list of files.
        self._list_view = ListBox(
            Widget.FILL_FRAME,
            model.get_summary(),
            name="selected_file_id",
            add_scroll_bar=True,
            on_change=self._on_pick,
            on_select=self._edit)
        self._edit_button = Button("Edit", self._edit)
        self._db_button = Button("DB queries", self._database)
        layout = Layout([100], fill_frame=True)
        self.add_layout(layout)
        layout.add_widget(self._list_view)
        layout.add_widget(Divider())
        layout2 = Layout([1, 1, 1, 1])
        self.add_layout(layout2)
        layout2.add_widget(Button("Create", self._add), 0)
        layout2.add_widget(self._edit_button, 1)
        layout2.add_widget(self._db_button, 2)
        layout2.add_widget(Button("Exit", self._quit), 3)
        self.fix()
        self._on_pick()

    def _on_pick(self):
        self._edit_button.disabled = self._list_view.value is None
        self._db_button.disabled = self._list_view.value is None

    def _reload_list(self, new_value=None):
        self._list_view.options = self._model.get_summary()
        self._list_view.value = new_value

    def _add(self):
        self._model.current_id = None
        raise NextScene("Create vCard")

    def _edit(self):
        self.save()
        self._model.current_id = self.data['selected_file_id']
        raise NextScene("Edit vCard")

    def _database(self):
        raise NextScene("Database")

    @staticmethod
    def _quit():
        sys.exit(0)

class CreateView(Frame):
    def __init__(self, screen, model):
        # Save off the model that accesses the contacts database.
        self._model = model

        super(CreateView, self).__init__(screen,
                                          screen.height * 2 // 3,
                                          screen.width * 2 // 3,
                                          hover_focus=True,
                                          can_scroll=False,
                                          title="vCard Details",
                                          reduce_cpu=True)

        # Create the form for editing/creating a new vCard file.
        layout = Layout([100], fill_frame=True)
        self.add_layout(layout)
        self._fileName_text = Text("File Name:", "file_name")
        self._contact_text = Text("Contact:", "contact")
        self._bday_text = Text("Birthday:", "bday")
        self._ann_text = Text("Anniversary:", "ann")
        self._op_text = Text("Other Properties:", "otherProp")
        self._bday_text.disabled = True
        self._ann_text.disabled = True
        self._op_text.disabled = True
        layout.add_widget(self._fileName_text)
        layout.add_widget(self._contact_text)
        layout.add_widget(self._bday_text)
        layout.add_widget(self._ann_text)
        layout.add_widget(self._op_text)
        layout2 = Layout([1, 1, 1, 1])
        self.add_layout(layout2)
        layout2.add_widget(Button("OK", self._ok), 0)
        layout2.add_widget(Button("Cancel", self._cancel), 3)
        self.fix()

    def reset(self):
        # Do standard reset to clear out form, then populate with new data.
        super(CreateView, self).reset()
        self._fileName_text.value = ""
        self._contact_text.value = ""
        self._bday_text.value = ""
        self._ann_text.value = ""
        self._op_text.value = ""
        self.fix()

    def _ok(self):
        tempVal_name = self._fileName_text.value
        tempVal_contact = self._contact_text.value

        if not tempVal_name:
            self._fileName_text.value = "CANNOT BE EMPTY!"
            return
        # Check if filename has a valid extension
        if not (tempVal_name.endswith(".vcf") or tempVal_name.endswith(".vcard")):
            self._fileName_text.value = "INVALID FILE TYPE! Must end with .vcf or .vcard"
            return
        if not tempVal_contact:
            self._contact_text.value = "CANNOT BE EMPTY!"
            return

        if not self._model.get_file_id(tempVal_name):
            # Convert the python strings into C strings
            tempVal_name = "cards/" + tempVal_name
            cName = tempVal_name.encode('utf-8')
            cContact = tempVal_contact.encode('utf-8')

            # Call the createFN(char * fileName, char * fn) function and store the return integer in returnStatus
            returnStatus = vcParser.createFN(c_char_p(cName), c_char_p(cContact))

            # Check if returnStatus is not 1
            if returnStatus != 1:
                self._fileName_text.value = "SAVE FAILED! TRY AGAIN!"
                self._contact_text.value = "SAVE FAILED! TRY AGAIN!"
                return

            # Save into the database
            self._model.create_file(self._fileName_text.value)
            file_id = self._model.get_file_id(self._fileName_text.value)
            self._model.add_contact(self._contact_text.value, file_id)

            raise NextScene("Main")
        else:
            # Inform the user that file already exists
            self._fileName_text.value = "FILE ALREADY EXISTS!"
            self._contact_text.value = "FILE ALREADY EXISTS!"
            return

    @staticmethod
    def _cancel():
        raise NextScene("Main")

class EditView(Frame):
    def __init__(self, screen, model):
        # Save off the model that accesses the contacts database.
        self._model = model

        super(EditView, self).__init__(screen,
                                          screen.height * 2 // 3,
                                          screen.width * 2 // 3,
                                          hover_focus=True,
                                          can_scroll=False,
                                          title="vCard Details",
                                          reduce_cpu=True)

        # Create the form for creating/editing the vCard files.
        layout = Layout([100], fill_frame=True)
        self.add_layout(layout)
        self._fileName_text = Text("File Name:", "name")
        self._contact_text = Text("Contact:", "contact")
        self._bday_text = Text("Birthday:", "bday")
        self._ann_text = Text("Anniversary:", "ann")
        self._op_text = Text("Other Properties:", "otherProp")
        self._fileName_text.disabled = True
        self._bday_text.disabled = True
        self._ann_text.disabled = True
        self._op_text.disabled = True
        layout.add_widget(self._fileName_text)
        layout.add_widget(self._contact_text)
        layout.add_widget(self._bday_text)
        layout.add_widget(self._ann_text)
        layout.add_widget(self._op_text)
        layout2 = Layout([1, 1, 1, 1])
        self.add_layout(layout2)
        layout2.add_widget(Button("OK", self._ok), 0)
        layout2.add_widget(Button("Cancel", self._cancel), 3)

        self.fix()

    def reset(self):
        # Do standard reset to clear out form, then populate with new data.
        super(EditView, self).reset()

        if self._model.current_id:
            # Convert the python string to a c string
            file_name = self._model.get_file_name(self._model.current_id)
            self._fileName_text.value = file_name

            if file_name:
                file_name = "cards/" + file_name
                cfileName = file_name.encode('utf-8')

                # Set the return type of createValidateStr(char * fileName) to c_char_p
                vcParser.createValidateStr.restype = c_char_p

                # Call the createValidateStr and store the return return C string in cDetails
                cDetails = vcParser.createValidateStr(c_char_p(cfileName))

                # Convert the C string into a python string
                pDetails = cDetails.decode('utf-8')

                # Split the string by ;
                details_list = pDetails.split(';')

                # Set the contact value field
                self._contact_text.value = details_list[0]

                # Display in the following format is not empty: "Date: YYYYMMDD Time: HHMMSS" or "Date: YYYYMMDD Time: HHMMSS (UTC)"
                if details_list[1] != "":
                    bday_date_time_list = details_list[1].split('T')

                    newStr = "Date: " + bday_date_time_list[0] + " Time: "

                    if bday_date_time_list[1].endswith('Z'):
                        newStr = newStr + bday_date_time_list[1].rstrip("Z") + " (UTC)"
                    else:
                        newStr = newStr + bday_date_time_list[1]
                    
                    self._bday_text.value = newStr
                else:
                    # Set the birthday value field
                    self._bday_text.value = details_list[1] 

                # Display in the following format is not empty: "Date: YYYYMMDD Time: HHMMSS" or "Date: YYYYMMDD Time: HHMMSS (UTC)"
                if details_list[2] != "":
                    ann_date_time_list = details_list[2].split('T')

                    newStrAnn = "Date: " + ann_date_time_list[0] + " Time: "

                    if ann_date_time_list[1].endswith('Z'):
                        newStrAnn = newStrAnn + ann_date_time_list[1].rstrip("Z") + " (UTC)"
                    else:
                        newStrAnn = newStrAnn + ann_date_time_list[1]
                    
                    self._ann_text.value = newStrAnn
                else:
                    # Set the birthday value field
                    self._ann_text.value = details_list[2] 

                # Set the other properties value field
                self._op_text.value = details_list[3]

        self.fix()

    def _ok(self):
        tempVal_name = self._fileName_text.value
        tempVal_contact = self._contact_text.value

        if not tempVal_name:
            self._fileName_text.value = "CANNOT BE EMPTY!"
            return
        # Check if filename has a valid extension
        if not (tempVal_name.endswith(".vcf") or tempVal_name.endswith(".vcard")):
            self._fileName_text.value = "INVALID FILE TYPE! Must end with .vcf or .vcard"
            return
        if not tempVal_contact:
            self._contact_text.value = "CANNOT BE EMPTY!"
            return

        if self._model.current_id:
            # Convert the python string to a c string
            file_name = self._model.get_file_name(self._model.current_id)
            self._fileName_text.value = file_name

            if file_name:
                file_name = "cards/" + file_name
                cfileName = file_name.encode('utf-8')
                cContact = tempVal_contact.encode('utf-8')

                # Call the createFN(char * fileName, char * fn) function and store the return integer in returnStatus
                returnStatus = vcParser.editFN(c_char_p(cfileName), c_char_p(cContact))

                # Check if returnStatus is not 1
                if returnStatus != 1:
                    self._fileName_text.value = "SAVE FAILED! TRY AGAIN!"
                    self._contact_text.value = "SAVE FAILED! TRY AGAIN!"
                    return

                # Update the database
                self._model.update_file(self._model.current_id)
                self._model.update_contact(self._contact_text.value, self._model.current_id)

        raise NextScene("Main")

    @staticmethod
    def _cancel():
        raise NextScene("Main")

class DatabaseView(Frame):
    def __init__(self, screen, model):
        super(DatabaseView, self).__init__(screen,
                                       screen.height * 2 // 3,
                                       screen.width * 2 // 3,
                                       on_load=self._reload_list,
                                       hover_focus=True,
                                       can_scroll=False)
        # Save off the model that accesses the contacts database.
        self._model = model

        # Create the form for displaying the queries.
        self._list_view = ListBox(
            Widget.FILL_FRAME,
            [],
            name="name",
            add_scroll_bar=True)
        layout = Layout([100], fill_frame=True)
        self.add_layout(layout)
        layout.add_widget(self._list_view)
        layout.add_widget(Divider())
        layout2 = Layout([1, 1, 1, 1, 1])
        self.add_layout(layout2)
        layout2.add_widget(Button("Display All", self._displayAll), 0)
        layout2.add_widget(Button("Born in June", self._bornInJune), 2)
        layout2.add_widget(Button("Cancel", self._cancel), 4)
        self.fix()

    def _reload_list(self, new_value=None):
        self._list_view.options = []
        self._list_view.value = new_value

    def _displayAll(self):
        # Fetch all contacts from the database
        contacts = self._model.get_all_contacts()  # This returns a list of tuples (contact data)
        num_rows_file = self._model.count_rows_file()
        num_rows_contact = self._model.count_rows_contact()

        # Create a list of formatted strings for the ListBox
        listbox_data = []
        datebase_stats = f"Database has {num_rows_file} files {num_rows_contact} contacts"
        listbox_data.append((datebase_stats, None))
        for contact in contacts:
            contact_str = f"ID: {contact[0]} | Name: {contact[1]} | Birthday: {contact[2]} | Anniversary: {contact[3]} | FILE NAME: {contact[4]}"
            listbox_data.append((contact_str, contact))  # Store the formatted string and the full contact tuple

        # Pass the formatted data to the ListBox
        self._list_view.options = listbox_data
        self._list_view.value = None
        self.fix()

    def _bornInJune(self):
        # Fetch all contacts from the database and number of rows for each table
        contacts = self._model.born_in_june()  # This returns a list of tuples (contact data)
        num_rows_file = self._model.count_rows_file()
        num_rows_contact = self._model.count_rows_contact()

        # Create a list of formatted strings for the ListBox
        listbox_data = []
        datebase_stats = f"Database has {num_rows_file} files {num_rows_contact} contacts"
        listbox_data.append((datebase_stats, None))
        for contact in contacts:
            contact_str = f"Name: {contact[0]} | Birthday: {contact[1]}"
            listbox_data.append((contact_str, contact))  # Store the formatted string and the full contact tuple

        # Pass the formatted data to the ListBox
        self._list_view.options = listbox_data
        self._list_view.value = None
        self.fix()

    @staticmethod
    def _cancel():
        raise NextScene("Main")

class LoginView(Frame):
    def __init__(self, screen, model):
        super(LoginView, self).__init__(screen,
                                          screen.height * 2 // 3,
                                          screen.width * 2 // 3,
                                          hover_focus=True,
                                          can_scroll=False,
                                          title="Login",
                                          reduce_cpu=True)
        # Save off the model that accesses the contacts database.
        self._model = model

        # Create the form for connecting to the database.
        layout = Layout([100], fill_frame=True)
        self.add_layout(layout)
        self._username_text = Text("Username:", "user")
        self._password_text = Text("Password", "password")
        self._dpName_text = Text("Database Name:", "dbName")
        layout.add_widget(self._username_text)
        layout.add_widget(self._password_text)
        layout.add_widget(self._dpName_text)
        layout2 = Layout([1, 1, 1, 1])
        self.add_layout(layout2)
        layout2.add_widget(Button("OK", self._ok), 0)
        layout2.add_widget(Button("Cancel", self._quit), 3)
        self.fix()

    def _ok(self):
        # Get the input
        dbName = self._dpName_text.value.strip()
        uName = self._username_text.value.strip() 
        passwd = self._password_text.value.strip()

        # Make sure the input is not empty
        if not dbName:
            self._dpName_text.value = "CANNOT BE EMPTY!"
            return
        if not uName:
            self._username_text.value = "CANNOT BE EMPTY!"
            return
        if not passwd:
            self._password_text.value = "CANNOT BE EMPTY!"
            return

        # Try connecting using a try catch
        try:
            # Try the connection
            conn = mysql.connector.connect(host="dursley.socs.uoguelph.ca", database=dbName, user=uName, password=passwd)

            # Set contacts._db to conn (connection made) and set conn.autocommit to True
            contacts._db = conn
            conn.autocommit = True

            # Store the create table commands in temp variables and execute them
            fileTable = "CREATE TABLE IF NOT EXISTS FILE (file_id INT AUTO_INCREMENT, file_name VARCHAR(60) NOT NULL, last_modified DATETIME, creation_time DATETIME NOT NULL, PRIMARY KEY (file_id))"
            contacts._db.cursor().execute(fileTable)
            contactTable = "CREATE TABLE IF NOT EXISTS CONTACT (contact_id INT AUTO_INCREMENT, name VARCHAR(256) NOT NULL, birthday DATETIME, anniversary DATETIME, file_id INT NOT NULL, PRIMARY KEY (contact_id), FOREIGN KEY (file_id) REFERENCES FILE (file_id) ON DELETE CASCADE)"
            contacts._db.cursor().execute(contactTable)

            # Call the get_files_from_folder function with the folder that has all the files to populate the database
            contacts.get_files_from_folder("cards/")

        except mysql.connector.Error as err:
            # Inform the user that the connection failed
            self._username_text.value = "CONNECTION FAILED! TRY AGAIN!"
            self._password_text.value = "CONNECTION FAILED! TRY AGAIN!"
            self._dpName_text.value = "CONNECTION FAILED! TRY AGAIN!"
            return
        
        # Login successful, now load and replace the current scenes
        scenes = [
            Scene([ListView(self.screen, contacts)], -1, name="Main"),
            Scene([CreateView(self.screen, contacts)], -1, name="Create vCard"),
            Scene([EditView(self.screen, contacts)], -1, name="Edit vCard"),
            Scene([DatabaseView(self.screen, contacts)], -1, name="Database"),
        ]

        # Play the scenes, set start_scene as the actual scene object
        self.screen.play(scenes, stop_on_resize=True, start_scene=scenes[0], allow_int=True)

        # Move on
        raise NextScene("Main")

    @staticmethod
    def _quit():
        raise StopApplication("User pressed quit")
        

def demo(screen, scene):
    scenes = [
        Scene([LoginView(screen, contacts)], -1, name="Login")
    ]
    
    # Play the scene
    screen.play(scenes, stop_on_resize=True, start_scene=scene, allow_int=True)

contacts = ContactModel()
last_scene = None
while True:
    try:
        Screen.wrapper(demo, catch_interrupt=True, arguments=[last_scene])
        sys.exit(0)
    except ResizeScreenError as e:
        last_scene = e.scene
