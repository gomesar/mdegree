# encoding : utf-8
from builtins import print
from sklearn.feature_extraction.text import TfidfTransformer
from sklearn.feature_extraction.text import CountVectorizer
from sklearn.ensemble import RandomForestClassifier
from sklearn.linear_model import LogisticRegression
from sklearn.model_selection import GridSearchCV
from nltk.stem.lancaster import LancasterStemmer
from sklearn.naive_bayes import MultinomialNB
from sklearn.datasets import load_files
from sklearn.decomposition import PCA
from nltk.corpus import stopwords
from os.path import isfile, join
from sklearn.svm import SVC
from os import listdir
import numpy as np
import random
import pickle
import string

"""
    conversao de caracteres maiusculos para minusculos [Checked]
    remoçao de pontuaçao [Checked]
    remoçao de stop words [Checked]
    steming dos termos [Checked]
    remocao dos termos que aparecem em um so documento [Checked]
"""
path = "/home/gomes/Workspaces/Unicamp/atv6/"
save_path = path + "save/"
N_FOLDS_HYP = 3


class Atv6:
    def __init__(self):
        self.save_id = self.check_saves()
        # Data
        self.data_bunch = None
        self.processed_words = None
        self.tokenized_b = None  # This is the one
        self.tokenized_freq = None  # This is the another one
        # Tools
        self.count_vect = CountVectorizer(lowercase=False, min_df=2)  # Discards words with frequency < 2
        self.count_vect_binary = CountVectorizer(lowercase=False, min_df=2, binary=True)  # Same but  binary
        self.tfidf_transformer = TfidfTransformer()
        self.pca = PCA(n_components=0.99)

        # Load or new
        self.load_brute_files()
        if not self.load_processed_files():
            self.processed_words = self.pre_process_text(self.data_bunch.data)
            self.tokenize()

        # extras
        self.tr_i = None
        self.tst_i = None
        self.gen_holdout_idx()
        self.svm_grid = {
            'C': [2 ** 5, 2 ** 8],
            'gamma': [2 ** -7, 2 ** -5, 2 ** 5],
            'kernel': ['rbf']
        }
        self.rfc_grid = {
            'max_features': [2**6, 2**8, 2**9, 2**10],
            'n_estimators': [200, 250, 300, 400]
        }

    @staticmethod
    def check_saves():
        saved_files = [f for f in listdir(save_path) if isfile(join(save_path, f))]
        return len(saved_files)+1

    @staticmethod
    def load_atv6(file_name):
        try:
            with open("{0}{1}".format(save_path, file_name), 'rb') as pickle_file:
                atv6 = pickle.load(pickle_file)
            return atv6
        except ValueError:
            print("[!] Can't retrieve processed words")
            raise

    def load_brute_files(self):
        try:
            print("Searching for path:\n{0}".format(path + "data/texts/"))
            word_files = load_files(path + "data/texts/")
            self.data_bunch = word_files
            self.processed_words = word_files.data
            return True
        except ValueError:
            print("[!] Can't open files.")
            raise

    def load_processed_files(self):
        try:
            with open("{0}{1}".format(save_path, "tokenized_b.dt"), 'rb') as pickle_file:
                self.tokenized_b = pickle.load(pickle_file)
            with open("{0}{1}".format(save_path, "tokenized_freq.dt"), 'rb') as pickle_file:
                self.tokenized_freq = pickle.load(pickle_file)

            return True
        except ValueError:
            print("[!] Can't load processed files")
            raise

    def save_atv6(self, mark):
        try:
            file_name = "{0}-atv6.dt".format(mark)
            pickle.dump(self, open(save_path + file_name, "wb"))
        except ValueError:
            print("[!] Can't save Atv6.")
            raise
        
    def save_data(self, data, mark):
        try:
            file_name = "{0}-{1}.dt".format(mark, self.save_id)
            pickle.dump(data, open(save_path + file_name, "wb"))
            self.save_id += 1
        except ValueError:
            print("[!] Can't save file.")
            raise

    # ###########################################################################
    # PROCESSING ################################################################
    @staticmethod
    def pre_process_text(word_list):
        stemmer = LancasterStemmer()
        stops = stopwords.words("english")
        bans = set(stops + list(string.punctuation))

        filtered_words = []
        for word in word_list:
            if word not in bans:
                lowered = str(word).lower()
                if not lowered.isnumeric():
                    filtered_words.append(stemmer.stem(lowered))
                else:
                    filtered_words.append(lowered)
        return filtered_words

    def tokenize(self):
        self.tokenized_b = self.count_vect_binary.fit_transform(self.processed_words)
        tokenized = self.count_vect.fit_transform(self.processed_words)
        self.tokenized_freq = self.tfidf_transformer.fit_transform(tokenized)

    def gen_holdout_idx(self):
        idx = range(0, 5000)
        self.tr_i = random.sample(idx, 4000)
        self.tst_i = list(set(idx) - set(self.tr_i))

    def get_holdout_b(self):
        x_train = self.tokenized_b[self.tr_i]
        x_test = self.tokenized_b[self.tst_i]

        y_train = self.data_bunch.target[self.tr_i]
        y_test = self.data_bunch.target[self.tst_i]

        return x_train, x_test, y_train, y_test

    def get_holdout_freq(self):
        x_train = self.tokenized_freq[self.tr_i]
        x_test = self.tokenized_freq[self.tst_i]

        y_train = self.data_bunch.target[self.tr_i]
        y_test = self.data_bunch.target[self.tst_i]

        return x_train, x_test, y_train, y_test

    def part2(self):
        print("------------------------------ Part II")
        #  Dividir a base de dados 4k treino, 1k test
        # x_train, x_test, y_train, y_test = train_test_split(self.tokenized_b, self.data_bunch.target, test_size=0.2)
        x_train, x_test, y_train, y_test = self.get_holdout_b()

        # Naive Bayes (Binary)
        nb = MultinomialNB()
        nb.fit(x_train, y_train)
        nb_score = nb.score(x_test, y_test)

        # Logist Regression (Binary)
        lr = LogisticRegression(C=10000, n_jobs=2)
        lr.fit(x_train, y_train)
        lr_score = lr.score(x_test, y_test)

        # Logistic Regression (Freq)
        x_train, x_test, y_train, y_test = self.get_holdout_freq()
        lr.fit(x_train, y_train)
        lr_score = lr.score(x_test, y_test)

        print("NB score (Binary): ", nb_score)
        print("LR score (Binary): ", lr_score)
        print("LR score (Frequency): ", lr_score)

    def do_pca(self):
        #  PCA com 99% variancia
        try:
            with open("{0}{1}".format(save_path, "pca-data.dt"), 'rb') as pickle_file:
                data_freq_pca = pickle.load(pickle_file)
            with open("{0}{1}".format(save_path, "pca-class.dt"), 'rb') as pickle_file:
                self.pca = pickle.load(pickle_file)
            if data_freq_pca.any():
                print("PCA and PCA transformed data loaded")
                print("", np.shape(data_freq_pca))
            else:
                print("Calculating PCA and transforming data...")

                data_freq_pca = self.pca.fit_transform(self.tokenized_freq.toarray())
                print("PCA-> X: ", sum(self.pca.explained_variance_ratio_))
                print("", np.shape(data_freq_pca))
                self.save_data(data_freq_pca, "pca")
        except ValueError:
            print("[!] Can't load/calculate PCA")
            raise

        x_train = data_freq_pca[self.tr_i]
        x_test = data_freq_pca[self.tst_i]

        y_train = self.data_bunch.target[self.tr_i]
        y_test = self.data_bunch.target[self.tst_i]
        dataset = {'xtrain': x_train, 'ytrain': y_train, 'xtest': x_test, 'ytest': y_test}
        return data_freq_pca, dataset

    def do_svc(self, x_train, y_train, x_test, y_test):
        #  Ao menos dois entre: SVM com RBF, gradient boosting e random forest. Na reduzida
        clf_gs_svc = GridSearchCV(SVC(), self.svm_grid, cv=N_FOLDS_HYP, n_jobs=3)
        clf_gs_svc.fit(x_train, y_train)
        svc_score = clf_gs_svc.score(x_test, y_test)
        print("Best svr params: ", clf_gs_svc.best_params_)

        return svc_score, clf_gs_svc

    def do_rfc(self, x_train, y_train, x_test, y_test):
        clf_gs_rfc = GridSearchCV(RandomForestClassifier(), self.rfc_grid, cv=N_FOLDS_HYP, n_jobs=3)
        clf_gs_rfc.fit(x_train, y_train)
        rfc_score = clf_gs_rfc.score(x_test, y_test)
        print("Best rfc params: ", clf_gs_rfc.best_params_)

        return rfc_score, clf_gs_rfc

    def part3(self):
        print("------------------------------ Part III")
        data_freq_pca = self.do_pca()

        # Split data
        x_train = data_freq_pca[self.tr_i]
        x_test = data_freq_pca[self.tst_i]

        y_train = self.data_bunch.target[self.tr_i]
        y_test = self.data_bunch.target[self.tst_i]
        # Do classification
        svc_score = self.do_svc(x_train, y_train, x_test, y_test)
        rfc_score = self.do_rfc(x_train, y_train, x_test, y_test)

        print("------------------------------")
        print("SVR Score: ", svc_score)
        print("RFC Score: ", rfc_score)

# Execution
if __name__ == "__main__":
    execution = Atv6()
    print("Starting.")
    execution.part2()
    execution.part3()
    print("End.")
else:
    print("Atv6 imported.")
